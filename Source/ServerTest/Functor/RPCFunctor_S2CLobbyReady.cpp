// Fill out your copyright notice in the Description page of Project Settings.


#include "RPCFunctor_S2CLobbyReady.h"
#include "../RPCPacketTypes.h"
#include "../Handlers/RPCPacketBase.h"
#include "../Handlers/RPCPacketWrapper.h"
#include "ServerTest/ServerTestPlayerController.h"

int32 FRPCFunctor_S2CLobbyReady::Execute_Implements(AServerTestPlayerController* inContext, const FRPCPacketWrapper& inWrapper)
{
	if (false == IsValid(inContext))
		return 1;

	if (ERPCPacketTypes::S2C_Lobby_Ready != inWrapper.PacketType)
		return 3;

	FMemoryReader Reader(inWrapper.Payload, true);
	TSharedPtr<FRPCPacket_S2C> packetS2C = MakeShared<FRPCPacket_S2C>();
	if (false == packetS2C.IsValid())
		return 3;

	Packet = packetS2C;
	return 0;
}