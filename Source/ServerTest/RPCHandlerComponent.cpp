// Fill out your copyright notice in the Description page of Project Settings.


#include "RPCHandlerComponent.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "RPCRequestManager.h"
#include "ServerTestPlayerController.h"

#define REGISTER_HANDLER(EnumType, MemberFunc) \
{ \
	FunctionMap.Emplace(EnumType, [this](const FRPCPacketWrapper& Wrapper) \
	{	\
		(this->*(&MemberFunc))(Wrapper); \
	}); \
}

#define REGISTER_HANDLER_VALIDATE(EnumType, ValidateMemberFunc) \
{ \
    FunctionMap_Validate.Emplace(EnumType, [this](const FRPCPacketWrapper& Wrapper) -> bool \
    {   \
        return (this->*(&ValidateMemberFunc))(Wrapper); \
    }); \
}

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
	FunctionMap.Reset();
	FunctionMap_Validate.Reset();
}

void URPCHandlerComponent::Initialized()
{
	Clear();

	// ��Ŷ ����
	REGISTER_HANDLER(EPacketType::ChangeColor, URPCHandlerComponent::OnReq_ChangeColor);

	// ��Ŷ ����
	REGISTER_HANDLER_VALIDATE(EPacketType::ChangeColor, URPCHandlerComponent::OnReq_ChangeColor_Validate);
}

// Ŭ���̾�Ʈ�� ������ ��Ŷ�� ���� �� ȣ��
void URPCHandlerComponent::Server_SendPacket_Implementation(FRPCPacketWrapper inWrapper)
{
	EPacketType type = inWrapper.PacketType;
	if (false == FunctionMap.Contains(type))
		return;

	const RPCHandler::HandlerDelegate* delegate = FunctionMap.Find(type);

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

bool URPCHandlerComponent::Server_SendPacket_Validate(FRPCPacketWrapper inWrapper)
{
	EPacketType type = inWrapper.PacketType;
	// ������ �ʿ��Ѱ�?
	if (false == FunctionMap_Validate.Contains(type))
		return true; // ��ϵ� ���� ������ ����.

	const RPCHandler::HandlerDelegate_Validate* delegate = FunctionMap_Validate.Find(type);
	if (delegate && (*delegate)) // TFunction�� IsBound() ��� operator bool() �� üũ ����
	{
		// ã�� TFunction (��������Ʈ) ����!
		if (false == (*delegate)(inWrapper))
		{
			FString reason = TEXT("kr");
			// ���� ���� �����ϱ�.
			SavedCheaterInfo(reason, inWrapper);
			return false;
		}

		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("No valid handler found for packet type: %d"), (int32)type);
	return false;
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
	FRPCPacketRes_Error errorResponse;
	errorResponse.SerialNumber = reqPacket.SerialNumber; // ���� ��û�� SerialNumber
	errorResponse.TimeStamp = FDateTime::UtcNow().ToUnixTimestamp(); // ������ �ð�.
	errorResponse.ResponseCode = inErrorCode; // ���� �ڵ�
	errorResponse.OriginalRequestType = inPacketWrapper.PacketType; // ���� ��û�� Ÿ��!
	errorResponse.ErrorMessage = TEXT("�������� �����Ͽ� ���� ���濡 �����߽��ϴ�."); // ������ �޽���

	FRPCPacketWrapper errorWrapper;
	errorWrapper.PacketType = EPacketType::Error_Response; // ���� ���� ���� Ÿ��
	if (errorWrapper.SerializePacket(errorResponse))
	{
		Client_ReceivePacket(errorWrapper);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to serialize GenericError response packet!"));
	}
}

void URPCHandlerComponent::SavedCheaterInfo(const FString& inReason, FRPCPacketWrapper inWrapper)
{
	// ���� ����! ������ ��� ���´�. �α״� ���⿡ ������.
	FString description = TEXT("UnknownPlayer");
	FString netAddress = TEXT("UnknownAddress");

	APlayerController* pc = Cast<APlayerController>(GetOwner());
	if (false == IsValid(pc))
		return;

	APlayerState* ps = GetPlayerState();
	if (IsValid(ps))
	{
		//description = FString::Printf(TEXT("PlayerName: %s, UniqueID: %s"),
		//	*ps->GetPlayerName(), 
		//	ps->GetUniqueId().IsValid() ? *ps->GetUniqueId().ToString() : TEXT("Invalid"));
	}

	UNetConnection* connection = GetNetConnection();
	if (IsValid(connection))
	{
		//netAddress = connection->LowLevelGetRemoteAddress(true);
	}

	// �α� ä���� ������ ����ų�, ���� ���� �α� ������ ����� �� �ֽ��ϴ�.
	//UE_LOG(LogSecurity, Warning, TEXT("Suspicious RPC activity from Client [%s, IP: %s]. Reason: '%s'. PacketType: %s."),
	//	*description, *netAddress, *inReason, *UEnum::GetValueAsString(inWrapper.PacketType) // EPacketType�� UENUM���� ������ٸ�
	//);
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
