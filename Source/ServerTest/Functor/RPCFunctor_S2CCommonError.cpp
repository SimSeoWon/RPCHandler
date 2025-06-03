// Fill out your copyright notice in the Description page of Project Settings.


#include "RPCFunctor_S2CCommonError.h"
#include "ServerTest/ServerTestPlayerController.h"
#include "../Handlers/RPCPacketWrapper.h"
#include "../RPCPacketTypes.h"
#include "../Handlers/RPCPacketBase.h"

int32 FRPCFunctor_S2CCommonError::Execute_Implements(AServerTestPlayerController* inContext, const FRPCPacketWrapper& inWrapper)
{
	if (false == IsValid(inContext))
		return 1;

	if (ERPCPacketTypes::S2C_Common_Error != inWrapper.PacketType)
		return 3;

	FMemoryReader Reader(inWrapper.Payload, true);
	TSharedPtr<FRPCPacketS2C_Error> packetS2C = MakeShared<FRPCPacketS2C_Error>();
	if (false == packetS2C.IsValid())
		return 3;

	packetS2C->SerializePacket(Reader);
	//�ҵ� ���¸� ó���ؾ� �Ѵ�.
	//packetS2C->SerialNumber
	//packetS2C.OriginalRequestType
	//packetS2C.SerialNumber

	//���� ������Ʈ�� �����ϱ�
	SerialNumber = packetS2C->SerialNumber;
	return packetS2C->ErrorCode; // ������ ���� ���� �ڵ� �����ϱ�.
}