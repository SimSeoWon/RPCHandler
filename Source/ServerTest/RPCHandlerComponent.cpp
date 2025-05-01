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
    
    /* TFunction에 this 컨텍스트와 함께 람다 또는 멤버 함수 바인딩 */

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
	// 스택에 할당한다.
	FRPCPacketReq_ChangeColor reqPacket;
	FMemoryReader reader(inPacketWrapper.Payload, true);
	reader.Seek(0);
	reqPacket.SerializePacket(reader); // Reader에서 데이터 읽어 Packet 채우기

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

// 클라이언트가 서버로 패킷을 보낼 때 호출
void URPCHandlerComponent::Server_SendPacket_Implementation(FRPCPacketWrapper inWrapper)
{
	EPacketType type = inWrapper.PacketType;
	if (false == DelegateFunctionMap.Contains(type))
		return;

	const RPCHandler::HandlerDelegate* delegate = DelegateFunctionMap.Find(type);

	// 핸들러를 찾았고, 유효한 함수가 바인딩되어 있는지 확인
	if (delegate && (*delegate)) // TFunction은 IsBound() 대신 operator bool() 로 체크 가능
	{
		// 찾은 TFunction (델리게이트) 실행!
		(*delegate)(inWrapper);
	}
	else
	{
		// 등록되지 않았거나 유효하지 않은 핸들러
		UE_LOG(LogTemp, Warning, TEXT("No valid handler found for packet type: %d"), (int32)type);
	}
}

bool URPCHandlerComponent::Server_SendPacket_Validate(FRPCPacketWrapper PacketWrapper)
{
	return true;
}

// 서버가 클라이언트로 패킷을 보낼 때 호출
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

