// Fill out your copyright notice in the Description page of Project Settings.


#include "RPCHandlerComponent.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "RPCRequestManager.h"
#include "ServerTestPlayerController.h"

#define REGISTER_HANDLER_DELEGATE(EnumType, MemberFunc) \
{ \
	DelegateFunctionMap.Emplace(EnumType, [this](const FRPCPacketWrapper& Wrapper) \
	{	\
		(this->*(&MemberFunc))(Wrapper); \
	}); \
}
    
    /* TFunction�� this ���ؽ�Ʈ�� �Բ� ���� �Ǵ� ��� �Լ� ���ε� */

// Sets default values for this component's properties
URPCHandlerComponent::URPCHandlerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicatedByDefault(true);
}

void URPCHandlerComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

// Called when the game starts
void URPCHandlerComponent::BeginPlay()
{
	Super::BeginPlay();

	DelegateFunctionMap.Reset();
	REGISTER_HANDLER_DELEGATE(EPacketType::ChangeColor, URPCHandlerComponent::OnReq_ChangeColor);
	// ...

}

void URPCHandlerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DelegateFunctionMap.Reset();
	Super::EndPlay(EndPlayReason);
	// ...
}

// Called every frame
void URPCHandlerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void URPCHandlerComponent::OnReq_ChangeColor(FRPCPacketWrapper inPacketWrapper)
{
	// ���ÿ� �Ҵ��Ѵ�.
	FRPCPacketReq_ChangeColor reqPacket;
	FMemoryReader reader(inPacketWrapper.Payload, true);
	reader.Seek(0);
	reqPacket.SerializePacket(reader); // Reader���� ������ �о� Packet ä���

	int32 playerNumber = reqPacket.PlayerNumber;
	FGuid serialNumber = reqPacket.SerialNumber;
	uint64 timeStamp = reqPacket.TimeStamp;

	FRPCPacketRes_ChangeColor resPacket;
	resPacket.ResponseCode = 0;
	resPacket.SerialNumber = serialNumber;
	resPacket.TimeStamp = timeStamp;

	FRPCPacketWrapper resPacketWrapper;
	resPacketWrapper.PacketType = EPacketType::ChangeColor_Response;
	resPacketWrapper.SerializePacket(resPacket);

	Client_ReceivePacket(resPacketWrapper);
}

// Ŭ���̾�Ʈ�� ������ ��Ŷ�� ���� �� ȣ��
void URPCHandlerComponent::Server_SendPacket_Implementation(FRPCPacketWrapper inWrapper)
{
	EPacketType type = inWrapper.PacketType;
	if (false == DelegateFunctionMap.Contains(type))
		return;

	const RPCHandler::HandlerDelegate* delegate = DelegateFunctionMap.Find(type);

	// �ڵ鷯�� ã�Ұ�, ��ȿ�� �Լ��� ���ε��Ǿ� �ִ��� Ȯ��
	if (delegate && (*delegate)) // TFunction�� IsBound() ��� operator bool() �� üũ ����
	{
		// ã�� TFunction (��������Ʈ) ����!
		(*delegate)(inWrapper);
	}
	else
	{
		// ��ϵ��� �ʾҰų� ��ȿ���� ���� �ڵ鷯
		UE_LOG(LogTemp, Warning, TEXT("No valid handler found for packet type: %d"), (int32)type);
	}
}

bool URPCHandlerComponent::Server_SendPacket_Validate(FRPCPacketWrapper PacketWrapper)
{
	return true;
}

// ������ Ŭ���̾�Ʈ�� ��Ŷ�� ���� �� ȣ��
void URPCHandlerComponent::Client_ReceivePacket_Implementation(FRPCPacketWrapper PacketWrapper)
{
	URPCRequestManager* manager = URPCRequestManager::Get(this);
	if (false == IsValid(manager))
		return;

	manager->RecvResponse(PacketWrapper);
}

AServerTestPlayerController* URPCHandlerComponent::FindPlayerControllerById(int32 inPlayerId)
{
	AGameStateBase* gameState = GetWorld()->GetGameState();
	if (false == IsValid(gameState))
		return nullptr;

	AServerTestPlayerController* controller = nullptr;

	for (auto iter : gameState->PlayerArray)
	{
		if (false == IsValid(iter))
			continue;

		if (iter->GetPlayerId() != inPlayerId)
			continue;

		controller = Cast<AServerTestPlayerController>(iter->GetOwner());
		break;
	}

	return controller;
}

