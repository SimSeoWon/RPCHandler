// Copyright Epic Games, Inc. All Rights Reserved.

#include "ServerTestGameMode.h"
#include "ServerTestPlayerController.h"
#include "ServerTestCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "ServerTestGameState.h"

#include "ServerTest.h"

AServerTestGameMode::AServerTestGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = AServerTestPlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/TopDown/Blueprints/BP_TopDownCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	// set default controller to our Blueprinted controller
	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBPClass(TEXT("/Game/TopDown/Blueprints/BP_TopDownPlayerController"));
	if(PlayerControllerBPClass.Class != NULL)
	{
		PlayerControllerClass = PlayerControllerBPClass.Class;
	}

	GameStateClass = AServerTestGameState::StaticClass();
}

void AServerTestGameMode::PreLoginAsync(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, const FOnPreLoginCompleteDelegate& OnComplete)
{
	Super::PreLoginAsync(Options, Address, UniqueId, OnComplete);
    TEST_LOG(LogServerTest, Log, TEXT("%s"), TEXT("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"));
	TEST_LOG(LogServerTest, Log, TEXT("%s"), TEXT("Begin"));
}

APlayerController* AServerTestGameMode::Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
    APlayerController* pc = Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage);

    TEST_LOG(LogServerTest, Log, TEXT("%s"), TEXT("Begin"));

    return pc;
}

void AServerTestGameMode::PostLogin(APlayerController* NewPlayer)
{
    TEST_LOG(LogServerTest, Log, TEXT("%s"), TEXT("Begin"));
    Super::PostLogin(NewPlayer);

	UNetDriver* netDriver = GetNetDriver();
	if (IsValid(netDriver))
	{
        if (0 >= netDriver->ClientConnections.Num())
        {
            TEST_LOG(LogServerTest, Log, TEXT("%s"), TEXT("Connections is Empty!"));
		}
        else
        {
			for (const auto& iter : netDriver->ClientConnections)
			{
				if (IsValid(iter))
				{
					TEST_LOG(LogServerTest, Log, TEXT("Connection Name : %s"), *iter->GetName());
				}
				else
				{
					TEST_LOG(LogServerTest, Log, TEXT("%s"), TEXT("Connection is not valid"));
				}
			}
        }

	}
    else
    {
        TEST_LOG(LogServerTest, Log, TEXT("%s"), TEXT("netDriver is not valid"));
    }

    TEST_LOG(LogServerTest, Log, TEXT("%s"), TEXT("End"));
    

    //AMyPlayerState* PlayerState = NewPlayer->GetPlayerState<AMyPlayerState>();
    //if (PlayerState)
    //{
    //    UE_LOG(LogTemp, Log, TEXT("Player %s has joined the game!"), *PlayerState->GetPlayerName());
    //}

    //// 플레이어 스폰
    //APawn* NewPawn = GetWorld()->SpawnActor<AMyCharacter>(DefaultPawnClass, FVector::ZeroVector, FRotator::ZeroRotator);
    //if (NewPawn)
    //{
    //    NewPlayer->Possess(NewPawn);
    //}
}

void AServerTestGameMode::StartPlay()
{
    TEST_LOG(LogServerTest, Log, TEXT("%s"), TEXT("Begin"));
    Super::StartPlay();
    TEST_LOG(LogServerTest, Log, TEXT("%s"), TEXT("End"));
}

void AServerTestGameMode::Logout(AController* Exiting)
{
    TEST_LOG(LogServerTest, Log, TEXT("%s"), TEXT("Begin"));
    Super::Logout(Exiting);
    TEST_LOG(LogServerTest, Log, TEXT("%s"), TEXT("End"));
    /*APlayerState* PlayerState = Exiting->GetPlayerState<APlayerState>();
    if (PlayerState)
    {
        UE_LOG(LogTemp, Log, TEXT("Player %s has left the game!"), *PlayerState->GetPlayerName());
    }*/
}