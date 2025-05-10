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
	// 에러코드를 받았다면...?
	if (inWrapper.PacketType == EPacketType::Error_Response)
	{
		FRPCPacketRes_Error errorResponse;
		FMemoryReader reader(inWrapper.Payload, true);
		errorResponse.SerializePacket(reader);

		// 로그를 남기고..
		UE_LOG(LogTemp, Warning, TEXT("Received Generic Error. Original Req: %s, Code: %d, Msg: %s"),
			*UEnum::GetValueAsString(errorResponse.OriginalRequestType),
			errorResponse.ResponseCode,
			*errorResponse.ErrorMessage);

		if (false == PendingRequests.Contains(errorResponse.SerialNumber))
			return; // 타임 아웃등으로 이미 제거됨.

		PendingRequests[errorResponse.SerialNumber].Callback(errorResponse);
		// 콜백 호출 (콜백 함수 내부에서 처리하자.)
		
		PendingRequests.Remove(errorResponse.SerialNumber);
	}
	else 
	{
		FRPCPacket_S2C response;
		FMemoryReader reader(inWrapper.Payload, true);
		response.SerializePacket(reader);

		if (false == PendingRequests.Contains(response.SerialNumber))
			return; // 타임 아웃등으로 이미 제거됨.

		//PendingRequests[response.SerialNumber].Callback(errorResponse);
		// 콜백 처리

		PendingRequests.Remove(response.SerialNumber);
		// Pending 목록에서 제거.
	}
}
