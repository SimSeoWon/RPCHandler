// Fill out your copyright notice in the Description page of Project Settings.


#include "RPCPacketWrapper.h"
#include "Serialization/BufferArchive.h"
#include "../RPCPacketTypes.h"
#include "RPCPacketBase.h"

bool FRPCPacketWrapper::SerializePacket(const FRPCPacketBase& inPacket)
{
	FBufferArchive archive;
	const_cast<FRPCPacketBase&>(inPacket).SerializePacket(archive);
	Payload = archive;

	return false == Payload.IsEmpty();
}