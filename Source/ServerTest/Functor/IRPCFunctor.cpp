// Fill out your copyright notice in the Description page of Project Settings.


#include "IRPCFunctor.h"
#include "ServerTest/ServerTestPlayerController.h"
#include "../RPCHandlerComponent.h"
#include "../Handlers/RPCPacketBase.h"
#include "../Handlers/RPCPacketWrapper.h"
#include "../RPCRequestManager.h"
#include "../RPCPacketTypes.h"

int32 IRPCFunctor::Execute(UObject* inContext, const FRPCPacketWrapper& inWrapper)
{
	Packet = nullptr; // 포인트 정리.
	if (false == IsValid(inContext))
		return 1;

	AServerTestPlayerController* pc = Cast<AServerTestPlayerController>(inContext);
	if (false == IsValid(pc))
		return 2;

	// 실제 패킷 처리 구현부
	int32 resultCode = Execute_Implements(pc, inWrapper);

	// 서버에서 보낸 패킷들이라면?
	if (ERPCPacketTypes::C2S_Maximum < inWrapper.PacketType)
	{
		URPCRequestManager* manager = URPCRequestManager::Get(inContext);
		if (IsValid(manager))
		{

			if (ERPCPacketTypes::S2C_Common_Error == inWrapper.PacketType)
			{
				///Todo - 등록된 팬딩 리스트를 삭제..
				manager->OnReceivedError(Packet, resultCode);
			}
			else 
			{
				///Todo - 등록된 팬딩 리스트에서 콜백처리...
				manager->OnReceivedResponse(Packet, resultCode);
			}
		}
	}
	
	// 여기서 매니저에게 콜백 함수를 처리할 수 있도록...
	return resultCode;
}
