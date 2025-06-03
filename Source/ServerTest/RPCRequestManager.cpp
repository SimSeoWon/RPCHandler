// Fill out your copyright notice in the Description page of Project Settings.


#include "RPCRequestManager.h"
#include "RPCHandlerComponent.h"
#include "Handlers/RPCPacketBase.h"
#include "RPCPacketTypes.h"
#include "Handlers/RPCPacketBase.h"
#include "../ServerTestPlayerController.h"


URPCRequestManager* URPCRequestManager::Get(const UObject* inWorldContext, int32 inPlayerIndex)
{
	return GetSubSystem<URPCRequestManager>(inWorldContext, inPlayerIndex);
}

void URPCRequestManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 타임아웃 검사 타이머 시작
	UWorld* world = GetWorld(); // ULocalPlayerSubsystem은 GetWorld()를 통해 World에 접근 가능
	if (world)
	{
		world->GetTimerManager().SetTimer(
			TimeoutCheckTimerHandle,      // 타이머 핸들
			this,                         // 호출될 함수를 가진 객체
			&URPCRequestManager::CheckTimeouts, // 호출될 함수 포인터
			TimeoutCheckInterval,         // 호출 간격 (초)
			true                          // 반복 여부
		);

		UE_LOG(LogTemp, Log, TEXT("URPCRequestManager: Timeout check timer started."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("URPCRequestManager: Could not get World to start timeout timer."));
	}
}

void URPCRequestManager::Deinitialize()
{
	// 핸들러 정리
	UWorld* world = GetWorld();
	if (world)
	{
		world->GetTimerManager().ClearTimer(TimeoutCheckTimerHandle);
		UE_LOG(LogTemp, Log, TEXT("URPCRequestManager: Timeout check timer cleared."));
	}

	PendingRequests.Reset();
	Super::Deinitialize();
}

void URPCRequestManager::CheckTimeouts()
{
	UWorld* world = GetWorld();
	if (false == IsValid(world))
		return;

	double CurrentTime = GetWorld()->GetTimeSeconds();
	TArray<FGuid> TimedOutRequestSerials;

	// 1. 타임아웃된 요청 찾기
	for (const auto& Pair : PendingRequests)
	{
		const FPendingRequest& Request = Pair.Value;
		if ((CurrentTime - Request.RequestTime) > RequestTimeoutDuration)
		{
			TimedOutRequestSerials.Add(Request.SerialNumber);
		}
	}

	// 2. 타임아웃된 요청 처리
	for (const FGuid& Serial : TimedOutRequestSerials)
	{
		if (FPendingRequest* TimedOutRequest = PendingRequests.Find(Serial))
		{
			UE_LOG(LogTemp, Warning, TEXT("Request timed out. Serial: %s"), *Serial.ToString());
			//Todo - 팝업창이라도 노출할까?
			PendingRequests.Remove(Serial); // 맵에서 제거
		}
	}
}

bool URPCRequestManager::SendRequest(ERPCPacketTypes type, FRPCPacket_C2S& inRequest, RPCHandler::OnResponseCallback inFunction)
{
	URPCHandlerComponent* rpcHandler = GetRPCHandlerComponent();
	if (false == IsValid(rpcHandler))
		return false;

	inRequest.SerialNumber = FGuid::NewGuid(); // SerialNumber 발급
	inRequest.TimeStamp = FDateTime::UtcNow().ToUnixTimestamp(); // 시간 기록
	inRequest.PlayerNumber = GetMyPlayerId();
	inRequest.Version = 1;

	// 실제로 서버에 전송하는 래퍼 클래스
	FRPCPacketWrapper wrapper;
	wrapper.SerializePacket(inRequest);
	wrapper.PacketType = type;

	// 큐에 등록
	FPendingRequest pending;
	pending.SerialNumber = inRequest.SerialNumber;
	pending.RequestTime = inRequest.TimeStamp;
	pending.Callback = inFunction;
	PendingRequests.Add(inRequest.SerialNumber, pending);

	///Todo - Send Packet To Server(Wrapper);
	rpcHandler->Server_SendPacket(wrapper);

	return true;
}

void URPCRequestManager::OnReceivedResponse(FGuid inSerialNumber, int32 resultCode)
{
	if (false == PendingRequests.Contains(inSerialNumber))// 타임 아웃등으로 이미 제거됨.
		return;
	
	if (0 >= resultCode) ///Todo - 에러가 없음.
	{
		// 등록된 콜백처리 시작.
		PendingRequests[inSerialNumber].Callback();
	}

	PendingRequests.Remove(inSerialNumber);
}

bool URPCRequestManager::Req_LobbyReady()
{
	FRPCPacket_C2S packet;
	TWeakObjectPtr<URPCRequestManager> WeakThis = this;
	auto lambda_OnReceived = [WeakThis]()->bool
		{
			if (false == WeakThis.IsValid())
				return false;

			return true;
		};

	return SendRequest(ERPCPacketTypes::C2S_Lobby_Ready, packet, lambda_OnReceived);
}
