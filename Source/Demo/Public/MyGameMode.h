// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class DEMO_API AMyGameMode : public AGameMode
{
	GENERATED_BODY()

	virtual void PreInitializeComponents() override;

	virtual void DefaultTimer();

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 RoundTime;

	FTimerHandle TimerHandle_DefaultTimer;

	TArray<FString> PlayerNameList;

	TArray<int32> PlayerScoreList;

	void SetPlayerInfoList();
public:
	
	AMyGameMode(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(exec)
	void FinishMatch();

	virtual void Killed(AController* Killer, AController* KilledPlayer, const UDamageType* DamageType);

	void OnPlayerKilled(ACharacter* Player);

	UFUNCTION(BlueprintCallable)
	TArray<FString> GetPlayerNameList();

	UFUNCTION(BlueprintCallable)
	TArray<int32> GetPlayerScoreList();
};
