// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameState.h"
#include "Demo/Demo.h"
#include "Net/UnrealNetwork.h"
#include "MyPlayerController.h"
#include "MyPlayerState.h"
#include "Engine/World.h"

void AMyGameState::UpdateAllPlayersInfo()
{
	for (FConstPlayerControllerIterator iter = GetWorld()->GetPlayerControllerIterator(); iter; ++iter)
	{
		AMyPlayerController* TempPlayerController = Cast<AMyPlayerController>(iter->Get());
		if (TempPlayerController)
		{
			AMyPlayerState* TempPlayerState = TempPlayerController->GetPlayerState<AMyPlayerState>();
			if (TempPlayerState)
			{
				PlayerNameList.Add(TempPlayerState->MyPlayerName);
				PlayerScoreList.Add(TempPlayerState->getNumKills() * 2 - TempPlayerState->getNumDeaths());
			}
		}
	}
}

AMyGameState::AMyGameState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	RemainingTime = 0;
}

void AMyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyGameState, RemainingTime);
}

TArray<FString> AMyGameState::GetPlayerNameList()
{
	UpdateAllPlayersInfo();

	return PlayerNameList;
}

TArray<int32> AMyGameState::GetPlayerScoreList()
{
	UpdateAllPlayersInfo();

	return PlayerScoreList;
}
