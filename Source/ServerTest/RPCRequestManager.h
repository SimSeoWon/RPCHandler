// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../LPlayerSubSystemBase.h"
#include "RPCRequestManager.generated.h"

/**
 * 
 */

struct FRPCPacketBase;
struct FRPCPacket_S2C;
struct FRPCPacket_C2S;
struct FRPCPacketWrapper;
enum class ERPCPacketTypes : uint16;



namespace RPCHandler // 또는 URPCRequestManager 내부
{
    using OnResponseCallback = TFunction<bool()>;
};

struct FPendingRequest
{
    FGuid SerialNumber;
    double RequestTime = 0.0;

    RPCHandler::OnResponseCallback Callback;
};

UCLASS()
class SERVERTEST_API URPCRequestManager : public ULPlayerSubSystemBase
{
	GENERATED_BODY()

protected:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    void CheckTimeouts(); // 타임아웃을 검사하는 함수
    bool SendRequest(ERPCPacketTypes type, FRPCPacket_C2S& inRequest, RPCHandler::OnResponseCallback inFunction);
    

public:
    static URPCRequestManager* Get(const UObject* inWorldContext, int32 inPlayerIndex = 0);
    bool Req_LobbyReady();
    void OnReceivedResponse(FGuid inSerialNumber, int32 resultCode);
    

protected:
    TMap<FGuid, FPendingRequest> PendingRequests;
    const float TimeoutCheckInterval = 1.0f; // 타임아웃 검사 주기 (예: 1초)
    const float RequestTimeoutDuration = 30.0f; // 요청 타임아웃 시간 (예: 30초)
    FTimerHandle TimeoutCheckTimerHandle; // 타임아웃 검사용 타이머 핸들
    
};