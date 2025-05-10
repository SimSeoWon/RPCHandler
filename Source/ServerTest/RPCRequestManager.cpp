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

	Request.SerialNumber = FGuid::NewGuid(); // SerialNumber �߱�
	Request.TimeStamp = FDateTime::UtcNow().ToUnixTimestamp(); // �ð� ���
	Request.PlayerNumber = GetMyPlayerId();

	// ������ ������ �����ϴ� ���� Ŭ����
	FRPCPacketWrapper wrapper;
	wrapper.SerializePacket(Request);
	wrapper.PacketType = inType;

	// ť�� ���
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
	// �����ڵ带 �޾Ҵٸ�...?
	if (inWrapper.PacketType == EPacketType::Error_Response)
	{
		FRPCPacketRes_Error errorResponse;
		FMemoryReader reader(inWrapper.Payload, true);
		errorResponse.SerializePacket(reader);

		// �α׸� �����..
		UE_LOG(LogTemp, Warning, TEXT("Received Generic Error. Original Req: %s, Code: %d, Msg: %s"),
			*UEnum::GetValueAsString(errorResponse.OriginalRequestType),
			errorResponse.ResponseCode,
			*errorResponse.ErrorMessage);

		if (false == PendingRequests.Contains(errorResponse.SerialNumber))
			return; // Ÿ�� �ƿ������� �̹� ���ŵ�.

		PendingRequests[errorResponse.SerialNumber].Callback(errorResponse);
		// �ݹ� ȣ�� (�ݹ� �Լ� ���ο��� ó������.)
		
		PendingRequests.Remove(errorResponse.SerialNumber);
	}
	else 
	{
		FRPCPacket_S2C response;
		FMemoryReader reader(inWrapper.Payload, true);
		response.SerializePacket(reader);

		if (false == PendingRequests.Contains(response.SerialNumber))
			return; // Ÿ�� �ƿ������� �̹� ���ŵ�.

		//PendingRequests[response.SerialNumber].Callback(errorResponse);
		// �ݹ� ó��

		PendingRequests.Remove(response.SerialNumber);
		// Pending ��Ͽ��� ����.
	}
}
