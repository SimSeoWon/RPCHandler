// Fill out your copyright notice in the Description page of Project Settings.


#include "RPCFunctor_C2SLobbyReady.h"
#include "ServerTest/ServerTestPlayerController.h"
#include "../RPCPacketTypes.h"
#include "../Handlers/RPCPacketBase.h"
#include "../Handlers/RPCPacketWrapper.h"
#include "../RPCRequestManager.h"

int32 FRPCFunctor_C2SLobbyReady::Execute_Implements(AServerTestPlayerController* inContext, const FRPCPacketWrapper& inWrapper)
{
	if (false == IsValid(inContext))
		return 1;

	if (ERPCPacketTypes::C2S_Lobby_Ready != inWrapper.PacketType)
		return 3;
	
	FMemoryReader Reader(inWrapper.Payload, true);
	TSharedPtr<FRPCPacketC2S_OneParam_Int> packetC2S = MakeShared<FRPCPacketC2S_OneParam_Int>();
	if (false == packetC2S.IsValid())
		return 3;

	packetC2S->SerializePacket(Reader);
	int32 playerNumber = packetC2S->PlayerNumber;
	int64 time = packetC2S->TimeStamp;
	FGuid serialNumber = packetC2S->SerialNumber;
	bool isReady = 0 < packetC2S->value;

	SerialNumber = serialNumber;
	return 0;
}