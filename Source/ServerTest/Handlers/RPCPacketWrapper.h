#pragma once
#include "CoreMinimal.h"
#include "RPCPacketWrapper.generated.h"

enum class ERPCPacketTypes : uint16;
struct FRPCPacketBase;

USTRUCT()
struct FRPCPacketWrapper
{
	GENERATED_BODY()

public:
	UPROPERTY()
	ERPCPacketTypes PacketType;

	UPROPERTY()
	TArray<uint8> Payload;

public:
	bool SerializePacket(const FRPCPacketBase& inPacket);
};