// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "LPlayerSubSystemBase.generated.h"

class AServerTestPlayerController;
class URPCHandlerComponent;
/**
 *
 */
UCLASS()
class SERVERTEST_API ULPlayerSubSystemBase : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:

	template <class T, class = typename TEnableIf<TPointerIsConvertibleFromTo<T, ULPlayerSubSystemBase>::Value>::Type>
	static T* GetSubSystem(const UObject* inWorldContext, int32 inPlayerIndex = 0)
	{
		// inWorldContext 유효성 검사
		if (false == IsValid(inWorldContext))
			return nullptr;
		
		UWorld* world = GEngine->GetWorldFromContextObject(inWorldContext, EGetWorldErrorMode::LogAndReturnNull);
		if (false == IsValid(world))
			return nullptr;

		// GameplayStatics를 사용하여 해당 인덱스의 PlayerController 얻기
		APlayerController* playerController = UGameplayStatics::GetPlayerController(world, inPlayerIndex);
		if (false == IsValid(playerController))
			return nullptr;

		// PlayerController로부터 LocalPlayer 얻기
		ULocalPlayer* localPlayer = playerController->GetLocalPlayer();
		if (false == IsValid(localPlayer))
			return nullptr;

		return localPlayer->GetSubsystem<T>();
	}

protected:
	int32 GetMyPlayerId();

	AServerTestPlayerController* GetPlayerController();

	URPCHandlerComponent* GetRPCHandlerComponent();

};
