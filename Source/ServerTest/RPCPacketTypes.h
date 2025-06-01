#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "RPCPacketTypes.generated.h"

UENUM()
enum class ERPCPacketTypes : uint16
{
	NONE,
	C2S_Lobby_Ready,
	C2S_Maximum, // 클라에서 요청하는 패킷을 구분하자.
	S2C_Lobby_Ready,
	S2C_Common_Error,
	// ... 이후 추가 가능
	MAX,
};