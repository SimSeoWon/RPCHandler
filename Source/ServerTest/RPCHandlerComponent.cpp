// Fill out your copyright notice in the Description page of Project Settings.


#include "RPCHandlerComponent.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "RPCRequestManager.h"
#include "../ServerTestPlayerController.h"
#include "Handlers/RPCPacketBase.h"
#include "Functor/RPCFunctor_C2SLobbyReady.h"
#include "Functor/RPCFunctor_S2CLobbyReady.h"
#include "Functor/RPCFunctor_S2CCommonError.h"
#include "RPCPacketTypes.h"

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
	Initialized(); // Function ��� ����.
}

void URPCHandlerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Clear();
	Super::EndPlay(EndPlayReason);
	// ...
}

// Called every frame
void URPCHandlerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	// ...
}
void URPCHandlerComponent::Clear()
{
	FunctorMap.Reset();
	FunctorList.Reset();
}

void URPCHandlerComponent::Initialized()
{
	Clear();

	//C2S_Functor
	FunctorMap.Add(ERPCPacketTypes::C2S_Lobby_Ready, MakeShared<FRPCFunctor_C2SLobbyReady>());

	//S2C_Functor
	FunctorMap.Add(ERPCPacketTypes::S2C_Lobby_Ready, MakeShared<FRPCFunctor_S2CLobbyReady>());	
	FunctorMap.Add(ERPCPacketTypes::S2C_Common_Error, MakeShared<FRPCFunctor_S2CCommonError>());

	FunctorList.Reserve(FunctorMap.Num());
	FunctorMap.GenerateValueArray(FunctorList);	
}

// Ŭ���̾�Ʈ�� ������ ��Ŷ�� ���� �� ȣ��
void URPCHandlerComponent::Server_SendPacket_Implementation(FRPCPacketWrapper inWrapper)
{
	ERPCPacketTypes type = inWrapper.PacketType;

	// Ŭ���̾�Ʈ�� �۾� �� �� �ִ� ������ �ƴϴ�.
	if (ERPCPacketTypes::C2S_Maximum >= type)
	{
		ResponseError(inWrapper, -1);// ���� ��Ŷ�� ������.
		UE_LOG(LogTemp, Warning, TEXT("No valid handler found for packet type: %d"), (int32)type);
		return;
	}

	if (false == FunctorMap.Contains(type))
	{
		ResponseError(inWrapper, -1);// ���� ��Ŷ�� ������.
		UE_LOG(LogTemp, Warning, TEXT("No valid handler found for packet type: %d"), (int32)type);
		return;
	}

	const TSharedPtr<IRPCFunctor> functor = FunctorMap[type];
	if (false == functor.IsValid())
	{
		ResponseError(inWrapper, -1);// ���� ��Ŷ�� ������.
		return;
	}

	int32 resultCode = functor->Execute(this, inWrapper);
	if (0 < resultCode) // �������� �۾��� �Ұ����ߴ�.
	{
		ResponseError(inWrapper, resultCode);// ���� ��Ŷ�� ������.
		return;
	}
}

bool URPCHandlerComponent::Server_SendPacket_Validate(FRPCPacketWrapper inWrapper)
{
	ERPCPacketTypes type = inWrapper.PacketType;
	if (false == FunctorMap.Contains(type))
	{
		// ��ϵ� ���Ͱ� �������� ����. �ϴ� �����Ų��.
		return true;
	}

	const TSharedPtr<IRPCFunctor> functor = FunctorMap[type];
	if (false == functor.IsValid())
	{
		// ���� �����Ͱ� ��ȿ���� �ʴ�. �ϴ� �����Ų��.
		return true;
	}

	return functor->Validate(this, inWrapper);
}

APlayerState* URPCHandlerComponent::GetPlayerState()
{
	APlayerController* pc = Cast<APlayerController>(GetOwner());
	if (false == IsValid(pc))
		return nullptr;

	return pc->GetPlayerState<APlayerState>();
}

UNetConnection* URPCHandlerComponent::GetNetConnection()
{
	APlayerController* pc = Cast<APlayerController>(GetOwner());
	if (false == IsValid(pc))
		return nullptr;

	return  pc->GetNetConnection();
}

//���� ��������
void URPCHandlerComponent::ResponseError(FRPCPacketWrapper inPacketWrapper, int32 inErrorCode/*= 0**/)
{
	FRPCPacket_C2S reqPacket;
	FMemoryReader reader(inPacketWrapper.Payload, true);
	reader.Seek(0);
	reqPacket.SerializePacket(reader); // Reader���� ������ �о� Packet ä���

	// ***** ���� �� ���� ���� ���� ��Ŷ ���� *****
	FRPCPacketS2C_Error errorResponse;
	errorResponse.SerialNumber = reqPacket.SerialNumber; // ���� ��û�� SerialNumber
	errorResponse.TimeStamp = FDateTime::UtcNow().ToUnixTimestamp(); // ������ �ð�.
	errorResponse.ResponseCode = inErrorCode; // ���� �ڵ�
	errorResponse.OriginalRequestType = inPacketWrapper.PacketType; // ���� ��û�� Ÿ��!
	errorResponse.ErrorMessage = TEXT(""); // ������ �޽���

	FRPCPacketWrapper errorWrapper;
	errorWrapper.PacketType = ERPCPacketTypes::S2C_Common_Error; // ���� ���� ���� Ÿ��
	if (errorWrapper.SerializePacket(errorResponse))
	{
		Client_ReceivePacket(errorWrapper);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to serialize GenericError response packet!"));
	}
}

 //������ Ŭ���̾�Ʈ�� ��Ŷ�� ���� �� ȣ��
void URPCHandlerComponent::Client_ReceivePacket_Implementation(FRPCPacketWrapper inWrapper)
{
	ERPCPacketTypes type = inWrapper.PacketType;

	// ������ �� �� �ִ� �۾��� �ƴϴ�.
	if (ERPCPacketTypes::C2S_Maximum < type)
	{
		//�˾� �����ұ�?
		//�ƴϸ� �α׸� ����ұ�?
		UE_LOG(LogTemp, Warning, TEXT("No valid handler found for packet type: %d"), (int32)type);
		return;
	}

	if (false == FunctorMap.Contains(type))
	{
		//�˾� �����ұ�?
		//�ƴϸ� �α׸� ����ұ�?
		UE_LOG(LogTemp, Warning, TEXT("No valid handler found for packet type: %d"), (int32)type);
		return;
	}

	const TSharedPtr<IRPCFunctor> functor = FunctorMap[type];
	if (false == functor.IsValid())
	{
		//�˾� �����ұ�?
		//�ƴϸ� �α׸� ����ұ�?
		return;
	}

	int32 resultCode = functor->Execute(this, inWrapper);
	if (0 < resultCode) // Ŭ�󿡼� �۾��� �Ұ����ߴ�.
	{
		//�˾� �����ұ�?
		//�ƴϸ� �α׸� ����ұ�?
	}
}

bool URPCHandlerComponent::Client_ReceivePacket_Validate(FRPCPacketWrapper inWrapper)
{
	ERPCPacketTypes type = inWrapper.PacketType;
	if (false == FunctorMap.Contains(type))
	{
		// ��ϵ� ���Ͱ� �������� ����. �ϴ� �����Ų��.
		return true;
	}

	const TSharedPtr<IRPCFunctor> functor = FunctorMap[type];
	if (false == functor.IsValid())
	{
		// ���� �����Ͱ� ��ȿ���� �ʴ�. �ϴ� �����Ų��.
		return true;
	}

	// ���� �Ұ� ��Ŷ�� ��ŵǰ� �ִ�. ���ӻ翡 �α׸� ������.
	return functor->Validate(this, inWrapper);
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
