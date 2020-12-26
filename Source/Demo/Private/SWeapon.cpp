// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "Demo/Demo.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ASWeapon::ASWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComponent;

	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "Target";

	BaseDamage = 10;
	RateOfFire = 600;
	BulletSpread = 2.0f;

	SetReplicates(true);
}

// Called when the game starts or when spawned
void ASWeapon::BeginPlay()
{
	Super::BeginPlay();

	TimeBetweenShots = 60 / RateOfFire;
	
}

void ASWeapon::Fire()
{
	//bool bIsHitTarget = false;
	/*GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Fire!"));*/

	if (GetLocalRole() < ROLE_Authority)
	{
		ServerFire();
	}

	AActor* MyOwner = GetOwner();
	if (MyOwner) 
	{
		FVector EyeLocation;
		FRotator EyeRotator;
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotator);
		FVector ShotDirection = EyeRotator.Vector();
		float HalfRad = FMath::DegreesToRadians(BulletSpread);
		ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);
		FVector TraceEnd = EyeLocation + (ShotDirection * 10000);
		FHitResult Hit;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(MyOwner);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;
		QueryParams.bReturnPhysicalMaterial = true;

		FVector TraceEndPoint = TraceEnd;
		EPhysicalSurface SurfaceType = EPhysicalSurface::SurfaceType_Default;

		UGameplayStatics::PlaySoundAtLocation(GetWorld(), RifleFireSound, EyeLocation);

		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams))
		{
			AActor* HitActor = Hit.GetActor();
			
			//GEngine->AddOnScreenDebugMessage(1, 5.0f, FColor::Red, HitActor->GetName());
			//if (HitActor->GetName() == TEXT("BP_Target_2"))
			//	bIsHitTarget = true;

			//UGameplayStatics::ApplyPointDamage(HitActor, 20, EyeRotator.Vector(), Hit, MyOwner->GetInstigatorController(), this, DamageType);

			UParticleSystem* SelectedEffect = nullptr;
			SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

			float ActualDamage = BaseDamage;

			if (SurfaceType == SURFACE_FLESHVULNERABLE)
			{
				ActualDamage *= 4;
			}

			//UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, EyeRotator.Vector(), Hit, MyOwner->GetInstigatorController(), this, DamageType);
			UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, MyOwner->GetInstigatorController(), this, DamageType);

			PlayImpactEffects(SurfaceType, Hit.ImpactPoint);

			TraceEndPoint = Hit.ImpactPoint;
		}

		PlayFireEffects(TraceEndPoint);

		if (GetLocalRole() == ROLE_Authority)
		{
			HitScanTrace.BurstCounter++;
			HitScanTrace.TraceTo = TraceEndPoint;
			HitScanTrace.SurfaceType = SurfaceType;
		}
		LastFireTime = GetWorld()->TimeSeconds;
	}

	//return bIsHitTarget;
}

void ASWeapon::StartFire()
{
	float FirstDelay = FMath::Max(0.0f, LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds);
	GetWorldTimerManager().SetTimer(TimerHandleTimeBetweenShots, this, &ASWeapon::Fire, TimeBetweenShots, true, FirstDelay);
}

void ASWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandleTimeBetweenShots);
}

void ASWeapon::ServerFire_Implementation()
{
	Fire();
}

bool ASWeapon::ServerFire_Validate()
{
	return true;
}


void ASWeapon::PlayFireEffects(FVector TraceEnd)
{
	if (MuzzleEffect)
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComponent, MuzzleSocketName);
	if (TracerEffect)
	{
		FVector MuzzleLocation = MeshComponent->GetSocketLocation(MuzzleSocketName);
		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
		if (TracerComp)
		{
			TracerComp->SetVectorParameter(TracerTargetName, TraceEnd);
		}

	}
}

void ASWeapon::OnRep_HitScanTrace()
{
	PlayFireEffects(HitScanTrace.TraceTo);
	PlayImpactEffects(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);
}

void ASWeapon::PlayImpactEffects(EPhysicalSurface SurfaceType, const FVector& ImpactPoint)
{
	UParticleSystem* SelectedEffect = nullptr;

	switch (SurfaceType)
	{
	case SURFACE_FLESHDEFAULT:
	case SURFACE_FLESHVULNERABLE:
		SelectedEffect = FleshImpactEffect;
		break;
	default:
		SelectedEffect = DefaultImpactEffect;
		break;
	}

	if (SurfaceType == SURFACE_FLESHDEFAULT)
	{
		if (DefaultAttackSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), DefaultAttackSound, ImpactPoint);
		}
	}

	if (SurfaceType == SURFACE_FLESHVULNERABLE)
	{
		if (HeadAttackSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), HeadAttackSound, ImpactPoint);
		}
	}

	if (SelectedEffect) 
	{
		FVector MuzzleLocation = MeshComponent->GetSocketLocation(MuzzleSocketName);
		FVector ShotDirection = ImpactPoint - MuzzleLocation;
		ShotDirection.Normalize();

		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactPoint, ShotDirection.Rotation());
	}

}

// Called every frame
void ASWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASWeapon, HitScanTrace, COND_SkipOwner);
}



