// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerState.h"
#include "Net/UnrealNetwork.h"

AMyPlayerState::AMyPlayerState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	int32 NameTail = FMath::Rand();
	FString BaseName = FString("JiaoWaa");
	BaseName.AppendInt(NameTail);

	MyPlayerName = BaseName;
	NumKills = 0;
	NumDeaths = 0;
}

void AMyPlayerState::ScoreKill()
{
	NumKills++;
}

void AMyPlayerState::ScoreDeath()
{
	NumDeaths++;
}

int32 AMyPlayerState::getNumKills()
{
	return NumKills;
}

int32 AMyPlayerState::getNumDeaths()
{
	return NumDeaths;
}

void AMyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyPlayerState, NumKills);
	DOREPLIFETIME(AMyPlayerState, NumDeaths);
	DOREPLIFETIME(AMyPlayerState, MyPlayerName);
}

