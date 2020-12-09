// Copyright Epic Games, Inc. All Rights Reserved.

#include "DemoCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/InputComponent.h"
#include "SWeapon.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Animation/AnimMontage.h"
#include "Components/SHealthComponent.h"
#include "Demo/Demo.h"
#include "MyPlayerState.h"
#include "MyGameMode.h"

//////////////////////////////////////////////////////////////////////////
// ADemoCharacter

void ADemoCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (GetLocalRole() == ROLE_Authority)
	{
		FActorSpawnParameters SpawmParams;
		SpawmParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		CurrentWeapon = GetWorld()->SpawnActor<ASWeapon>(StartWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawmParams);

		if (CurrentWeapon)
		{
			CurrentWeapon->SetOwner(this);
			CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);
		}
	}

	HealthComp->OnHealthChanged.AddDynamic(this, &ADemoCharacter::OnHealthChanged);

	/*FActorSpawnParameters SpawmParams;
	SpawmParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	CurrentWeapon = GetWorld()->SpawnActor<ASWeapon>(StartWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawmParams);

	if (CurrentWeapon)
	{
		CurrentWeapon->SetOwner(this);
		CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);
	}*/
}


void ADemoCharacter::OnRep_Goal()
{
	OnGoalUpdate();
}

void ADemoCharacter::OnGoalUpdate()
{
	if (IsLocallyControlled())
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("CLIENT"));
	}

	if (GetLocalRole() == ROLE_Authority)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("SERVER"));
	}
}


void ADemoCharacter::OnHealthChanged(USHealthComponent* OwnerHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (bIsDead) return;
	if (Health <= 0 && !bIsDead)
	{
		bIsDead = true;
		GetMovementComponent()->StopMovementImmediately();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		GetCameraBoom()->bDoCollisionTest = false;
		CameraBoom->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, true));
		
		SetLifeSpan(5.0f);
		CurrentWeapon->SetLifeSpan(5.0f);
		Die(HealthDelta, DamageType, InstigatedBy, DamageCauser);

		GetWorldTimerManager().SetTimer(TimerHandleRebirth, this, &ADemoCharacter::Rebirth, 3.0f, false);
		
		/* ��Ϊ���в�����ͼ������Controller����ִ��ĳЩController��صĲ�����������ʱ�����������䣬����ͼ���ֵ��߼����������ټ���
		DetachFromControllerPendingDestroy();
		SetLifeSpan(3.0f);
		*/
	}
}

ADemoCharacter::ADemoCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));

	//SetReplicates(true);
	WeaponAttachSocketName = "WeaponSocket";
	Goal = 0;

	bIsDead = false;

	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);
}

//////////////////////////////////////////////////////////////////////////
// Input

void ADemoCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &ADemoCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ADemoCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ADemoCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ADemoCharacter::LookUpAtRate);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ADemoCharacter::OnResetVR);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ADemoCharacter::Fire);
}

void ADemoCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ADemoCharacter, ControllerRotation, COND_SkipOwner);

	DOREPLIFETIME(ADemoCharacter, CurrentWeapon);
	DOREPLIFETIME(ADemoCharacter, Goal);
	DOREPLIFETIME(ADemoCharacter, bIsDead);
}

void ADemoCharacter::SetGoal()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		++Goal;
		OnGoalUpdate();
	}
}

void ADemoCharacter::SetGoalBlueprintVersion(int32 LastTimeGoal)
{
	Goal = LastTimeGoal;
}


void ADemoCharacter::ThrowMontage(UAnimMontage* ThrowAnim)
{
	ThrowMontageServer(ThrowAnim);
}

void ADemoCharacter::SetCurrentWeapon()
{
	FActorSpawnParameters SpawmParams;
	SpawmParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (CurrentWeapon)
	{
		CurrentWeapon->Destroy();
	}

	CurrentWeapon = GetWorld()->SpawnActor<ASWeapon>(StartWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawmParams);

	if (CurrentWeapon)
	{
		CurrentWeapon->SetOwner(this);
		CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);
	}
}

bool ADemoCharacter::Die(float KillingDamage, const class UDamageType* DamageType, class AController* Killer, class AActor* DamageCauser)
{
	Killer = GetDamageInstigator(Killer, *DamageType);

	AController* const KilledPlayer = (Controller != NULL) ? Controller : Cast<AController>(GetOwner());
	
	if (GetWorld()->GetAuthGameMode<AMyGameMode>())
	{
		GetWorld()->GetAuthGameMode<AMyGameMode>()->Killed(Killer, KilledPlayer, DamageType);
	}

	return true;
}

void ADemoCharacter::Rebirth()
{
	GetWorldTimerManager().ClearTimer(TimerHandleRebirth);
	GetWorld()->GetAuthGameMode<AMyGameMode>()->OnPlayerKilled(this);
}

void ADemoCharacter::SetControllerRotation()
{
	if (GetLocalRole() == ROLE_Authority || IsLocallyControlled())
	{
		if (GetController() != NULL)
		{
			ControllerRotation = GetController()->GetControlRotation();
		}
	}
}

void ADemoCharacter::ThrowMontageMulticast_Implementation(UAnimMontage* ThrowAnim)
{
	PlayAnimMontage(ThrowAnim);
}

void ADemoCharacter::ThrowMontageServer_Implementation(UAnimMontage* ThrowAnim)
{
	ThrowMontageMulticast(ThrowAnim);
}

bool ADemoCharacter::ThrowMontageServer_Validate(UAnimMontage* ThrowAnim)
{
	return true;
}

void ADemoCharacter::HandleFire_Implementation()
{
	SetGoal();
}

void ADemoCharacter::Fire()
{
	if (CurrentWeapon)
	{
		bool isHitTarget = false;
		CurrentWeapon->StartFire();

		if (isHitTarget)
		{
			if (GetLocalRole() < ROLE_Authority)
			{
				HandleFire();
			}
			else
			{
				SetGoal();
			}
		}
	}
}

void ADemoCharacter::StopFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StopFire();
	}
}

void ADemoCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ADemoCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ADemoCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ADemoCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ADemoCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

FVector ADemoCharacter::GetPawnViewLocation() const
{
	if (FollowCamera)
	{
		return FollowCamera->GetComponentLocation();
	}
	return Super::GetPawnViewLocation();
}

