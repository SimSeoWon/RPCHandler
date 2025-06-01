#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "RPCPacketTypes.generated.h"

UENUM()
enum class ERPCPacketTypes : uint16
{
	NONE,
	C2S_Lobby_Ready,
	C2S_Maximum, // Ŭ�󿡼� ��û�ϴ� ��Ŷ�� ��������.
	S2C_Lobby_Ready,
	S2C_Common_Error,
	// ... ���� �߰� ����
	MAX,
};