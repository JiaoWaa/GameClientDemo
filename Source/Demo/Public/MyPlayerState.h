// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MyPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class DEMO_API AMyPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	void ScoreKill();
	
	void ScoreDeath();
	
protected:

	UPROPERTY(BlueprintReadOnly, Replicated)
	int32 NumKills;

	UPROPERTY(BlueprintReadOnly, Replicated)
	int32 NumDeaths;

public:

	AMyPlayerState(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable)
	int32 getNumKills();

	UFUNCTION(BlueprintCallable)
	int32 getNumDeaths();

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Player")
	FString MyPlayerName;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
