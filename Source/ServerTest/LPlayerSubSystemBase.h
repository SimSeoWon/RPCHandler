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
		// inWorldContext ��ȿ�� �˻�
		if (false == IsValid(inWorldContext))
			return nullptr;
		
		UWorld* world = GEngine->GetWorldFromContextObject(inWorldContext, EGetWorldErrorMode::LogAndReturnNull);
		if (false == IsValid(world))
			return nullptr;

		// GameplayStatics�� ����Ͽ� �ش� �ε����� PlayerController ���
		APlayerController* playerController = UGameplayStatics::GetPlayerController(world, inPlayerIndex);
		if (false == IsValid(playerController))
			return nullptr;

		// PlayerController�κ��� LocalPlayer ���
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
