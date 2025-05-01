// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LPlayerSubSystemBase.h"
#include "RPCRequestManager.generated.h"

/**
 * 
 */

struct FRPCPacket_S2C;
struct FRPCPacket_C2S;
struct FRPCPacketWrapper;
enum class EPacketType : uint16;

struct FPendingRequest
{
    FGuid SerialNumber;
    double RequestTime = 0.0;
    TFunction<void(const FRPCPacket_S2C&)> Callback;
};

UCLASS()
class SERVERTEST_API URPCRequestManager : public ULPlayerSubSystemBase
{
	GENERATED_BODY()

public:
    static URPCRequestManager* Get(const UObject* inWorldContext, int32 inPlayerIndex = 0);
    void SendRequest(EPacketType type, FRPCPacket_C2S& inRequest, TFunction<void(const FRPCPacket_S2C&)> inFunction);
    void RecvResponse(const FRPCPacketWrapper& inWrapper);
    void ReqChangeColor();
    
protected:
    TMap<FGuid, FPendingRequest> PendingRequests;
};
