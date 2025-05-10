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
	Initialized(); // Function 목록 수집.
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

	// 패킷 수신
	REGISTER_HANDLER(EPacketType::ChangeColor, URPCHandlerComponent::OnReq_ChangeColor);

	// 패킷 검증
	REGISTER_HANDLER_VALIDATE(EPacketType::ChangeColor, URPCHandlerComponent::OnReq_ChangeColor_Validate);
}

// 클라이언트가 서버로 패킷을 보낼 때 호출
void URPCHandlerComponent::Server_SendPacket_Implementation(FRPCPacketWrapper inWrapper)
{
	EPacketType type = inWrapper.PacketType;
	if (false == FunctionMap.Contains(type))
		return;

	const RPCHandler::HandlerDelegate* delegate = FunctionMap.Find(type);

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

bool URPCHandlerComponent::Server_SendPacket_Validate(FRPCPacketWrapper inWrapper)
{
	EPacketType type = inWrapper.PacketType;
	// 검증이 필요한가?
	if (false == FunctionMap_Validate.Contains(type))
		return true; // 등록된 검증 로직이 없음.

	const RPCHandler::HandlerDelegate_Validate* delegate = FunctionMap_Validate.Find(type);
	if (delegate && (*delegate)) // TFunction은 IsBound() 대신 operator bool() 로 체크 가능
	{
		// 찾은 TFunction (델리게이트) 실행!
		if (false == (*delegate)(inWrapper))
		{
			FString reason = TEXT("kr");
			// 상대방 정보 저장하기.
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

//에러 응답하자
void URPCHandlerComponent::ResponseError(FRPCPacketWrapper inPacketWrapper, int32 inErrorCode/*= 0**/)
{
	FRPCPacket_C2S reqPacket;
	FMemoryReader reader(inPacketWrapper.Payload, true);
	reader.Seek(0);
	reqPacket.SerializePacket(reader); // Reader에서 데이터 읽어 Packet 채우기

	// ***** 실패 시 공용 에러 응답 패킷 전송 *****
	FRPCPacketRes_Error errorResponse;
	errorResponse.SerialNumber = reqPacket.SerialNumber; // 원본 요청의 SerialNumber
	errorResponse.TimeStamp = FDateTime::UtcNow().ToUnixTimestamp(); // 응답한 시간.
	errorResponse.ResponseCode = inErrorCode; // 에러 코드
	errorResponse.OriginalRequestType = inPacketWrapper.PacketType; // 원본 요청의 타입!
	errorResponse.ErrorMessage = TEXT("아이템이 부족하여 색상 변경에 실패했습니다."); // 선택적 메시지

	FRPCPacketWrapper errorWrapper;
	errorWrapper.PacketType = EPacketType::Error_Response; // 공용 에러 응답 타입
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
	// 검증 실패! 접속을 즉시 끊는다. 로그는 여기에 남기자.
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

	// 로그 채널을 별도로 만들거나, 보안 관련 로그 레벨을 사용할 수 있습니다.
	//UE_LOG(LogSecurity, Warning, TEXT("Suspicious RPC activity from Client [%s, IP: %s]. Reason: '%s'. PacketType: %s."),
	//	*description, *netAddress, *inReason, *UEnum::GetValueAsString(inWrapper.PacketType) // EPacketType을 UENUM으로 만들었다면
	//);
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
