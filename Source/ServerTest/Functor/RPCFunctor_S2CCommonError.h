// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IRPCFunctor.h"

struct FRPCPacketWrapper;
/**
 * 
 */
class FRPCFunctor_S2CCommonError : public IRPCFunctor
{
public:
	

	// ���������� ��û ����
	virtual bool Validate(UObject* Context, const FRPCPacketWrapper& Wrapper) { return true; }

protected:
	// ���� RPC ó��
	virtual int32 Execute_Implements(AServerTestPlayerController* inContext, const FRPCPacketWrapper& inWrapper) override;

};
