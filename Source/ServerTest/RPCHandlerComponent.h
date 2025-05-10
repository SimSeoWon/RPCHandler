// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Serialization/BufferArchive.h"
#include "RPCHandlerComponent.generated.h"

class AServerTestPlayerController;

namespace RPCHandler
{
	// TFunction<void(const FRPCPacketWrapper&)> 타입에 대한 짧은 이름 정의
	using HandlerDelegate = TFunction<void(const FRPCPacketWrapper&)>;
	using HandlerDelegate_Validate = TFunction<bool(const FRPCPacketWrapper&)>;
}

UENUM()
enum class EPacketType : uint16
{
	NONE,
	ChangeColor,
	ChangeColor_Response,
	Error_Response, // 공용 에러 응답 타입 추가
	MAX,
};

USTRUCT()
struct FRPCPacketBase
{
	GENERATED_BODY()
public:
	UPROPERTY()
	FGuid SerialNumber; // 패킷 구분자

	UPROPERTY()
	uint64 TimeStamp = 0; //UTF-0 기준 표준시간. 

public:
	// 공통 직렬화 로직 (Overridable)
	virtual void SerializePacket(FArchive& inArchive)
	{
		inArchive << SerialNumber;
		inArchive << TimeStamp;
	}
};

USTRUCT()
struct FRPCPacket_C2S : public FRPCPacketBase ///Todo - 클라이언트가 서버에 요청함.
{
	GENERATED_BODY()
public:

	UPROPERTY()
	int32 PlayerNumber = -1; // 플레이어 아이디
	

	virtual void SerializePacket(FArchive& inArchive) override
	{
		FRPCPacketBase::SerializePacket(inArchive);
		inArchive << PlayerNumber;
	}
};

USTRUCT()
struct FRPCPacket_S2C : public FRPCPacketBase ///Todo - 서버가 클라이언트에게 응답함. 
{
	 GENERATED_BODY()
public:

	UPROPERTY()
	int32 ResponseCode = -1; // 성공, 실패, 에러 코드 등...
	
	virtual void SerializePacket(FArchive& inArchive) override
	{
		FRPCPacketBase::SerializePacket(inArchive);
		inArchive << ResponseCode;
	}
};

// 테스트 패킷
USTRUCT()
struct FRPCPacketReq_ChangeColor : public FRPCPacket_C2S
{
	GENERATED_BODY()
public:
	UPROPERTY()
	FLinearColor Color = FLinearColor::White; // 변경할 컬러 정보.

public:
	virtual void SerializePacket(FArchive& inArchive) override
	{
		FRPCPacket_C2S::SerializePacket(inArchive);
		inArchive << Color;
	}
};

// 테스트 패킷
USTRUCT()
struct FRPCPacketRes_ChangeColor : public FRPCPacket_S2C
{
	GENERATED_BODY()
public:
	///Todo : 응답받고 큐에서 제거하고... 흠....

public:
	virtual void SerializePacket(FArchive& inArchive) override
	{
		FRPCPacket_S2C::SerializePacket(inArchive);
		
	}
};

USTRUCT()
struct FRPCPacketRes_Error : public FRPCPacket_S2C
{
	GENERATED_BODY()
public:
	///Todo : 응답받고 큐에서 제거하고... 흠....
	   // 실패한 원본 요청의 패킷 타입
	UPROPERTY()
	EPacketType OriginalRequestType = EPacketType::NONE;

	// (선택 사항) 좀 더 상세한 에러 메시지 문자열
	UPROPERTY()
	FString ErrorMessage;

public:
	virtual void SerializePacket(FArchive& inArchive) override
	{
		FRPCPacket_S2C::SerializePacket(inArchive); // 부모 클래스 직렬화 (SerialNumber, TimeStamp, ResponseCode 처리)
		inArchive << OriginalRequestType;
		inArchive << ErrorMessage;
	}
};

USTRUCT()
struct FRPCPacketWrapper
{
	GENERATED_BODY()

public:
	UPROPERTY()
	EPacketType PacketType;

	UPROPERTY()
	TArray<uint8> Payload;

public:
	bool SerializePacket(const FRPCPacketBase& inPacket)
	{
		FBufferArchive archive;
		const_cast<FRPCPacketBase&>(inPacket).SerializePacket(archive);
		Payload = archive;

		return false == Payload.IsEmpty();
	}
};


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SERVERTEST_API URPCHandlerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	URPCHandlerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void InitializeComponent() override;

	
public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

#pragma region RPC core
protected:
	void Initialized();
	void Clear();
	void SavedCheaterInfo(const FString& inReason, FRPCPacketWrapper inWrapper);
	APlayerState* GetPlayerState();
	UNetConnection* GetNetConnection();
	void ResponseError(FRPCPacketWrapper inPacketWrapper, int32 inErrorCode = 0);

public:
	// 클라이언트가 서버로 패킷을 보낼 때 호출
	UFUNCTION(Server, Reliable, WithValidation) // 안정적인 전송 + 검증 함수 필요
		void Server_SendPacket(FRPCPacketWrapper PacketWrapper);

	bool Server_SendPacket_Validate(FRPCPacketWrapper PacketWrapper);
	void Server_SendPacket_Implementation(FRPCPacketWrapper PacketWrapper);

	// 서버가 클라이언트로 패킷을 보낼 때 호출
	UFUNCTION(Client, Reliable)
	void Client_ReceivePacket(FRPCPacketWrapper PacketWrapper);
	void Client_ReceivePacket_Implementation(FRPCPacketWrapper PacketWrapper);
#pragma endregion  RPC core

#pragma region server side processing
public:
	UFUNCTION()
	void OnReq_ChangeColor(FRPCPacketWrapper inPacketWrapper);

	UFUNCTION()
	bool OnReq_ChangeColor_Validate(FRPCPacketWrapper inPacketWrapper);

	

protected:
	//멤버 함수 포인터 사용
	TMap<EPacketType, RPCHandler::HandlerDelegate> FunctionMap;
	TMap<EPacketType, RPCHandler::HandlerDelegate_Validate> FunctionMap_Validate;
#pragma endregion  server side processing

protected:
	AServerTestPlayerController* FindPlayerControllerById(int32 inPlayerId);
};
