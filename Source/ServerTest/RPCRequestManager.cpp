// Fill out your copyright notice in the Description page of Project Settings.


#include "RPCRequestManager.h"

#include "RPCHandlerComponent.h"
#include "ServerTestPlayerController.h"



URPCRequestManager* URPCRequestManager::Get(const UObject* inWorldContext, int32 inPlayerIndex)
{
	return GetSubSystem<URPCRequestManager>(inWorldContext, inPlayerIndex);
}

///Todo - Test Packet
void URPCRequestManager::ReqChangeColor()
{

	auto lambda_RecvPack = [this](const FRPCPacket_S2C& inRes)
		{
			if (inRes.ResponseCode == 0)
			{
				return;
			}
		};

	FRPCPacketReq_ChangeColor packet;
	packet.Color = FLinearColor::Yellow;
	SendRequest(EPacketType::ChangeColor, packet, lambda_RecvPack);
}

void URPCRequestManager::SendRequest(EPacketType inType, FRPCPacket_C2S& Request, TFunction<void(const FRPCPacket_S2C&)> OnResponse)
{
	URPCHandlerComponent* rpcHandler = GetRPCHandlerComponent();
	if (false == IsValid(rpcHandler))
		return;

	Request.SerialNumber = FGuid::NewGuid(); // SerialNumber 발급
	Request.TimeStamp = FDateTime::UtcNow().ToUnixTimestamp(); // 시간 기록
	Request.PlayerNumber = GetMyPlayerId();

	// 실제로 서버에 전송하는 래퍼 클래스
	FRPCPacketWrapper wrapper;
	wrapper.SerializePacket(Request);
	wrapper.PacketType = inType;

	// 큐에 등록
	FPendingRequest pending;
	pending.SerialNumber = Request.SerialNumber;
	pending.RequestTime = Request.TimeStamp;
	pending.Callback = OnResponse;
	PendingRequests.Add(Request.SerialNumber, pending);

	///Todo - Send Packet To Server(Wrapper);
	rpcHandler->Server_SendPacket(wrapper);
}

void URPCRequestManager::RecvResponse(const FRPCPacketWrapper& inWrapper)
{

}
