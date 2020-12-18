// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCharacter.h"
#include "SWeapon.h"
#include "Components/SHealthComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"

AEnemyCharacter::AEnemyCharacter() : ADemoCharacter() { }

void AEnemyCharacter::BeginPlay()
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

	HealthComp->OnHealthChanged.Clear();
	HealthComp->OnHealthChanged.AddDynamic(this, &AEnemyCharacter::OnEnemyHealthChanged);
	
}

void AEnemyCharacter::OnEnemyHealthChanged(USHealthComponent* OwnerHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (bIsDead) return;
	if (Health <= 0 && !bIsDead)
	{
		bIsDead = true;
		GetMovementComponent()->StopMovementImmediately();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		SetLifeSpan(5.0f);
		if (CurrentWeapon)
		{
			CurrentWeapon->SetLifeSpan(5.0f);
		}
		Die(HealthDelta, DamageType, InstigatedBy, DamageCauser);

	}
}

float AEnemyCharacter::GetCurrentHealth()
{
	return HealthComp->GetHealth();
}
