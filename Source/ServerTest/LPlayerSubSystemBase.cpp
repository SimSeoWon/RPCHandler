// Fill out your copyright notice in the Description page of Project Settings.


#include "LPlayerSubSystemBase.h"
#include "GameFramework/PlayerState.h"
#include "ServerTestPlayerController.h"

int32 ULPlayerSubSystemBase::GetMyPlayerId()
{
	ULocalPlayer* localPlayer = GetLocalPlayer();
	if (false == IsValid(localPlayer))
		return -1;

	APlayerController* playerController = localPlayer->PlayerController;
	if (false == IsValid(playerController))
		return -1;

	APlayerState* playerState = playerController->PlayerState;
	if (false == IsValid(playerState))
		return -1;

	return playerState->GetPlayerId();
}


AServerTestPlayerController* ULPlayerSubSystemBase::GetPlayerController() 
{
	ULocalPlayer* localPlayer = GetLocalPlayer();
	if (false == IsValid(localPlayer))
		return nullptr;

	APlayerController* playerController = localPlayer->PlayerController;
	if (false == IsValid(playerController))
		return nullptr;

	return Cast<AServerTestPlayerController>(playerController);
}

URPCHandlerComponent* ULPlayerSubSystemBase::GetRPCHandlerComponent() 
{
	AServerTestPlayerController* playerController = GetPlayerController();
	if (false == IsValid(playerController))
		return nullptr;

	return playerController->GetRPCHandler();
}