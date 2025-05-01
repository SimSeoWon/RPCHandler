// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#define LOG_LOCALROLEINFO *(UEnum::GetValueAsString(TEXT("Engine.ENetRole"), GetLocalRole()))
#define LOG_REMOTEROLEINFO *(UEnum::GetValueAsString(TEXT("Engine.ENetRole"), GetRemoteRole()))
#define LOG_NETMODEINFO ((ENetMode::NM_Client == GetNetMode()) ?  *FString::Printf(TEXT("Client%d"), GPlayInEditorID) : (ENetMode::NM_Standalone == GetNetMode()) ? TEXT("Standalone") : TEXT("Server"))
#define LOG_CALLINFO ANSI_TO_TCHAR(__FUNCTION__)
#define TEST_LOG(LogCat, Verbosity, Fromat, ...) UE_LOG(LogCat, Verbosity, TEXT("[%s][%s/%s] %s %s"), LOG_NETMODEINFO, LOG_LOCALROLEINFO, LOG_REMOTEROLEINFO, LOG_CALLINFO, *FString::Printf(Fromat, ##__VA_ARGS__));

DECLARE_LOG_CATEGORY_EXTERN(LogServerTest, Log, All);



