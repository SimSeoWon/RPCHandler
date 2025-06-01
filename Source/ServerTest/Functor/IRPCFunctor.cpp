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
	Packet = nullptr; // ����Ʈ ����.
	if (false == IsValid(inContext))
		return 1;

	AServerTestPlayerController* pc = Cast<AServerTestPlayerController>(inContext);
	if (false == IsValid(pc))
		return 2;

	// ���� ��Ŷ ó�� ������
	int32 resultCode = Execute_Implements(pc, inWrapper);

	// �������� ���� ��Ŷ���̶��?
	if (ERPCPacketTypes::C2S_Maximum < inWrapper.PacketType)
	{
		URPCRequestManager* manager = URPCRequestManager::Get(inContext);
		if (IsValid(manager))
		{

			if (ERPCPacketTypes::S2C_Common_Error == inWrapper.PacketType)
			{
				///Todo - ��ϵ� �ҵ� ����Ʈ�� ����..
				manager->OnReceivedError(Packet, resultCode);
			}
			else 
			{
				///Todo - ��ϵ� �ҵ� ����Ʈ���� �ݹ�ó��...
				manager->OnReceivedResponse(Packet, resultCode);
			}
		}
	}
	
	// ���⼭ �Ŵ������� �ݹ� �Լ��� ó���� �� �ֵ���...
	return resultCode;
}
