// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"


void AMyPlayerController::GameHasEnded(class AActor* EndGameFocus /*= NULL*/, bool bIsWinner /*= false*/)
{
	Super::GameHasEnded(EndGameFocus, bIsWinner);
}

