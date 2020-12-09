// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class DEMO_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()
	

public:

	virtual void GameHasEnded(class AActor* EndGameFocus = NULL, bool bIsWinner = false) override;
};
