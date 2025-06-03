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
	//팬딩 상태를 처리해야 한다.
	//packetS2C->SerialNumber
	//packetS2C.OriginalRequestType
	//packetS2C.SerialNumber

	//게임 스테이트에 세팅하기
	SerialNumber = packetS2C->SerialNumber;
	return packetS2C->ErrorCode; // 서버가 보낸 에러 코드 전달하기.
}