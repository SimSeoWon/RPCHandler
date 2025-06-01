// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IRPCFunctor.h"
//#include "RPCFunctor_SetReady.generated.h"

struct FRPCPacketWrapper;
/**
 * 
 */
class FRPCFunctor_C2SLobbyReady : public IRPCFunctor
{
public:
	// 선택적으로 요청 검증
	virtual bool Validate(UObject* Context, const FRPCPacketWrapper& Wrapper) { return true; }

protected:
	// 실제 RPC 처리
	virtual int32 Execute_Implements(AServerTestPlayerController* Context, const FRPCPacketWrapper& Wrapper) override;

};
