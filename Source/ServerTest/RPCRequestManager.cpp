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

	// Ÿ�Ӿƿ� �˻� Ÿ�̸� ����
	UWorld* world = GetWorld(); // ULocalPlayerSubsystem�� GetWorld()�� ���� World�� ���� ����
	if (world)
	{
		world->GetTimerManager().SetTimer(
			TimeoutCheckTimerHandle,      // Ÿ�̸� �ڵ�
			this,                         // ȣ��� �Լ��� ���� ��ü
			&URPCRequestManager::CheckTimeouts, // ȣ��� �Լ� ������
			TimeoutCheckInterval,         // ȣ�� ���� (��)
			true                          // �ݺ� ����
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
	// �ڵ鷯 ����
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

	// 1. Ÿ�Ӿƿ��� ��û ã��
	for (const auto& Pair : PendingRequests)
	{
		const FPendingRequest& Request = Pair.Value;
		if ((CurrentTime - Request.RequestTime) > RequestTimeoutDuration)
		{
			TimedOutRequestSerials.Add(Request.SerialNumber);
		}
	}

	// 2. Ÿ�Ӿƿ��� ��û ó��
	for (const FGuid& Serial : TimedOutRequestSerials)
	{
		if (FPendingRequest* TimedOutRequest = PendingRequests.Find(Serial))
		{
			UE_LOG(LogTemp, Warning, TEXT("Request timed out. Serial: %s"), *Serial.ToString());
			//Todo - �˾�â�̶� �����ұ�?
			PendingRequests.Remove(Serial); // �ʿ��� ����
		}
	}
}

bool URPCRequestManager::SendRequest(ERPCPacketTypes type, FRPCPacket_C2S& inRequest, RPCHandler::OnResponseCallback inFunction)
{
	URPCHandlerComponent* rpcHandler = GetRPCHandlerComponent();
	if (false == IsValid(rpcHandler))
		return false;

	inRequest.SerialNumber = FGuid::NewGuid(); // SerialNumber �߱�
	inRequest.TimeStamp = FDateTime::UtcNow().ToUnixTimestamp(); // �ð� ���
	inRequest.PlayerNumber = GetMyPlayerId();
	inRequest.Version = 1;

	// ������ ������ �����ϴ� ���� Ŭ����
	FRPCPacketWrapper wrapper;
	wrapper.SerializePacket(inRequest);
	wrapper.PacketType = type;

	// ť�� ���
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
	if (false == PendingRequests.Contains(inSerialNumber))// Ÿ�� �ƿ������� �̹� ���ŵ�.
		return;
	
	if (0 >= resultCode) ///Todo - ������ ����.
	{
		// ��ϵ� �ݹ�ó�� ����.
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
