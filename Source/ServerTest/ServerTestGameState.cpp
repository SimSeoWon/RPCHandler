// Fill out your copyright notice in the Description page of Project Settings.


#include "ServerTestGameState.h"
#include "ServerTest.h"

void AServerTestGameState::HandleBeginPlay()
{
	Super::HandleBeginPlay();
	TEST_LOG(LogServerTest, Log, TEXT("%s"), TEXT("Begin"));
}

void AServerTestGameState::OnRep_ReplicatedHasBegunPlay()
{
	Super::OnRep_ReplicatedHasBegunPlay();
	TEST_LOG(LogServerTest, Log, TEXT("%s"), TEXT("Begin"));
}