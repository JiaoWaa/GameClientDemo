// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Demo/DemoCharacter.h"
#include "EnemyCharacter.generated.h"

/**
 * 
 */
UCLASS()
class DEMO_API AEnemyCharacter : public ADemoCharacter
{
	GENERATED_BODY()
	
public:
	AEnemyCharacter();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnEnemyHealthChanged(USHealthComponent* OwnerHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION(BlueprintCallable)
	float GetCurrentHealth();
};
