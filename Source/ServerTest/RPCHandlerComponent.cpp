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

// 클라이언트가 서버로 패킷을 보낼 때 호출
void URPCHandlerComponent::Server_SendPacket_Implementation(FRPCPacketWrapper inWrapper)
{
	ERPCPacketTypes type = inWrapper.PacketType;

	// 클라이언트가 작업 할 수 있는 영역이 아니다.
	if (ERPCPacketTypes::C2S_Maximum >= type)
	{
		ResponseError(inWrapper, -1);// 에러 패킷을 보내자.
		UE_LOG(LogTemp, Warning, TEXT("No valid handler found for packet type: %d"), (int32)type);
		return;
	}

	if (false == FunctorMap.Contains(type))
	{
		ResponseError(inWrapper, -1);// 에러 패킷을 보내자.
		UE_LOG(LogTemp, Warning, TEXT("No valid handler found for packet type: %d"), (int32)type);
		return;
	}

	const TSharedPtr<IRPCFunctor> functor = FunctorMap[type];
	if (false == functor.IsValid())
	{
		ResponseError(inWrapper, -1);// 에러 패킷을 보내자.
		return;
	}

	int32 resultCode = functor->Execute(this, inWrapper);
	if (0 < resultCode) // 서버에서 작업이 불가능했다.
	{
		ResponseError(inWrapper, resultCode);// 에러 패킷을 보내자.
		return;
	}
}

bool URPCHandlerComponent::Server_SendPacket_Validate(FRPCPacketWrapper inWrapper)
{
	ERPCPacketTypes type = inWrapper.PacketType;
	if (false == FunctorMap.Contains(type))
	{
		// 등록된 펑터가 존재하지 않음. 일단 통과시킨다.
		return true;
	}

	const TSharedPtr<IRPCFunctor> functor = FunctorMap[type];
	if (false == functor.IsValid())
	{
		// 펑터 포인터가 유효하지 않다. 일단 통과시킨다.
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

//에러 응답하자
void URPCHandlerComponent::ResponseError(FRPCPacketWrapper inPacketWrapper, int32 inErrorCode/*= 0**/)
{
	FRPCPacket_C2S reqPacket;
	FMemoryReader reader(inPacketWrapper.Payload, true);
	reader.Seek(0);
	reqPacket.SerializePacket(reader); // Reader에서 데이터 읽어 Packet 채우기

	// ***** 실패 시 공용 에러 응답 패킷 전송 *****
	FRPCPacketS2C_Error errorResponse;
	errorResponse.SerialNumber = reqPacket.SerialNumber; // 원본 요청의 SerialNumber
	errorResponse.TimeStamp = FDateTime::UtcNow().ToUnixTimestamp(); // 응답한 시간.
	errorResponse.ResponseCode = inErrorCode; // 에러 코드
	errorResponse.OriginalRequestType = inPacketWrapper.PacketType; // 원본 요청의 타입!
	errorResponse.ErrorMessage = TEXT(""); // 선택적 메시지

	FRPCPacketWrapper errorWrapper;
	errorWrapper.PacketType = ERPCPacketTypes::S2C_Common_Error; // 공용 에러 응답 타입
	if (errorWrapper.SerializePacket(errorResponse))
	{
		Client_ReceivePacket(errorWrapper);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to serialize GenericError response packet!"));
	}
}

 //서버가 클라이언트로 패킷을 보낼 때 호출
void URPCHandlerComponent::Client_ReceivePacket_Implementation(FRPCPacketWrapper inWrapper)
{
	ERPCPacketTypes type = inWrapper.PacketType;

	// 서버가 할 수 있는 작업이 아니다.
	if (ERPCPacketTypes::C2S_Maximum < type)
	{
		//팝업 노출할까?
		//아니면 로그를 등록할까?
		UE_LOG(LogTemp, Warning, TEXT("No valid handler found for packet type: %d"), (int32)type);
		return;
	}

	if (false == FunctorMap.Contains(type))
	{
		//팝업 노출할까?
		//아니면 로그를 등록할까?
		UE_LOG(LogTemp, Warning, TEXT("No valid handler found for packet type: %d"), (int32)type);
		return;
	}

	const TSharedPtr<IRPCFunctor> functor = FunctorMap[type];
	if (false == functor.IsValid())
	{
		//팝업 노출할까?
		//아니면 로그를 등록할까?
		return;
	}

	int32 resultCode = functor->Execute(this, inWrapper);
	if (0 < resultCode) // 클라에서 작업이 불가능했다.
	{
		//팝업 노출할까?
		//아니면 로그를 등록할까?
	}
}

bool URPCHandlerComponent::Client_ReceivePacket_Validate(FRPCPacketWrapper inWrapper)
{
	ERPCPacketTypes type = inWrapper.PacketType;
	if (false == FunctorMap.Contains(type))
	{
		// 등록된 펑터가 존재하지 않음. 일단 통과시킨다.
		return true;
	}

	const TSharedPtr<IRPCFunctor> functor = FunctorMap[type];
	if (false == functor.IsValid())
	{
		// 펑터 포인터가 유효하지 않다. 일단 통과시킨다.
		return true;
	}

	// 검증 불가 패킷이 통신되고 있다. 게임사에 로그를 남기자.
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
