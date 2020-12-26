// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameMode.h"
#include "TimerManager.h"
#include "MyGameState.h"
#include "MyPlayerController.h"
#include "MyPlayerState.h"
#include "EngineUtils.h"
#include "Engine.h"

AMyGameMode::AMyGameMode(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnOb(TEXT("/Game/Player/ThirdPersonCharacter"));
	DefaultPawnClass = PlayerPawnOb.Class;

	PlayerControllerClass = AMyPlayerController::StaticClass();
	PlayerStateClass = AMyPlayerState::StaticClass();

	static ConstructorHelpers::FClassFinder<AGameState> CurrentGameState(TEXT("/Game/ThirdPersonCPP/Blueprints/BP_MyGameState"));
	GameStateClass = CurrentGameState.Class;
	//GameStateClass = AMyGameState::StaticClass();

	RoundTime = 300;
	TimeBetweenMatches = 5;
}

void AMyGameMode::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	AMyGameState* const CurrentGameState = Cast<AMyGameState>(GameState);
	CurrentGameState->RemainingTime = RoundTime;

	GetWorldTimerManager().SetTimer(TimerHandle_DefaultTimer, this, &AMyGameMode::DefaultTimer, GetWorldSettings()->GetEffectiveTimeDilation(), true);
}

void AMyGameMode::HandleMatchIsWaitingToStart()
{
	AMyGameState* const CurrentGameState = Cast<AMyGameState>(GameState);
	CurrentGameState->RemainingTime = 2.0f;
}

void AMyGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	AMyGameState* const CurrentGameState = Cast<AMyGameState>(GameState);
	CurrentGameState->RemainingTime = RoundTime;
	CurrentGameState->CloseRankboard();
}

void AMyGameMode::DefaultTimer()
{
	AMyGameState* const CurrentGameState = Cast<AMyGameState>(GameState);

	/*if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, FString::FromInt(CurrentGameState->RemainingTime));
	}*/

	if (CurrentGameState && CurrentGameState->RemainingTime > 0)
	{
		CurrentGameState->RemainingTime--;

		if (CurrentGameState->RemainingTime <= 0)
		{
			if (GetMatchState() == MatchState::WaitingPostMatch)
			{
				/*if (GEngine)
				{
					GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Restart!!!!!!"));
				}*/
				RestartGame();

				GetWorld()->ServerTravel("/Game/ThirdPersonCPP/Maps/ShootMap");
			}
			else if (GetMatchState() == MatchState::InProgress)
			{
				CurrentGameState->ShowRankboard();
				FinishMatch();
			}
			else if (GetMatchState() == MatchState::WaitingToStart)
			{
				/*if (GEngine)
				{
					GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Start*****"));
				}*/
				StartMatch();
			}
		}
	}
}

void AMyGameMode::Killed(AController* Killer, AController* KilledPlayer, const UDamageType* DamageType)
{
	AMyPlayerState* KillerPlayerState = Killer ? Cast<AMyPlayerState>(Killer->PlayerState) : NULL;
	AMyPlayerState* VictimPlayerState = KilledPlayer ? Cast<AMyPlayerState>(KilledPlayer->PlayerState) : NULL;

	if (KillerPlayerState && KillerPlayerState != VictimPlayerState)
	{
		KillerPlayerState->ScoreKill();
	}

	if (VictimPlayerState)
	{
		VictimPlayerState->ScoreDeath();
	}
}

void AMyGameMode::OnPlayerKilled(ACharacter* Player)
{
	AMyPlayerController* KilledPlayerController = Cast<AMyPlayerController>(Player->GetController());

	if (KilledPlayerController)
	{
		KilledPlayerController->UnPossess();

		RestartPlayer(KilledPlayerController);
	}

}

void AMyGameMode::SetPlayerInfoList()
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

TArray<FString> AMyGameMode::GetPlayerNameList()
{
	SetPlayerInfoList();

	return PlayerNameList;
}

TArray<int32> AMyGameMode::GetPlayerScoreList()
{
	SetPlayerInfoList();

	return PlayerScoreList;
}

void AMyGameMode::FinishMatch()
{
	AMyGameState* const CurrentGameState = Cast<AMyGameState>(GameState);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Game Over"));
	}

	if (IsMatchInProgress())
	{
		EndMatch();
		
		for (FConstControllerIterator iter = GetWorld()->GetControllerIterator(); iter; ++iter)
		{
			(*iter)->GameHasEnded();
		}

		for (APawn* Pawn : TActorRange<APawn>(GetWorld()))
		{
			Pawn->TurnOff();
		}

		CurrentGameState->RemainingTime = TimeBetweenMatches;
	}

}
