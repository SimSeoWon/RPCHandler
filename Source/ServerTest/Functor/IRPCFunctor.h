// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

//#include "IRPCFunctor.generated.h"


struct FRPCPacketWrapper;
struct FRPCPacketBase;
class AServerTestPlayerController;
/**
 *
 */
class IRPCFunctor
{
public:
	virtual ~IRPCFunctor() = default;

	// 실제 RPC 처리
	int32 Execute(UObject* Context, const FRPCPacketWrapper& Wrapper);

	// 선택적으로 요청 검증
	virtual bool Validate(UObject* Context, const FRPCPacketWrapper& Wrapper) { return true; }

protected:
	virtual int32 Execute_Implements(AServerTestPlayerController* inContext, const FRPCPacketWrapper& inWrapper) = 0;

protected:
	TSharedPtr<FRPCPacketBase> Packet = nullptr;
};
