// Fill out your copyright notice in the Description page of Project Settings.


#include "SProjectileWeapon.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"

void ASProjectileWeapon::Fire()
{

	AActor* MyOwner = GetOwner();
	if (MyOwner)
	{
		FVector EyeLocation;
		FRotator EyeRotator;
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotator);
		
		FVector MuzzleLocation = MeshComponent->GetSocketLocation(MuzzleSocketName);
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		if (ProjectileClass)
		{
			GetWorld()->SpawnActor<AActor>(ProjectileClass, MuzzleLocation, EyeRotator, SpawnParams);
		}
		
	}

	//return false;
}