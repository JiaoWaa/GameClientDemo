// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "MyGameState.generated.h"

/**
 * 
 */
UCLASS()
class DEMO_API AMyGameState : public AGameState
{
	GENERATED_BODY()
	
protected:
	void UpdateAllPlayersInfo();

	TArray<FString> PlayerNameList;

	TArray<int32> PlayerScoreList;

public:
	AMyGameState(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(Replicated, BlueprintReadOnly)
	int32 RemainingTime;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable)
	TArray<FString> GetPlayerNameList();

	UFUNCTION(BlueprintCallable)
	TArray<int32> GetPlayerScoreList();

	UFUNCTION(BlueprintImplementableEvent)
	void ShowRankboard();

	UFUNCTION(BlueprintImplementableEvent)
	void CloseRankboard();
};
