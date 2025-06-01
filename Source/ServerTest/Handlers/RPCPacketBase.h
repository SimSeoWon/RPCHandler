#pragma once
#include "CoreMinimal.h"
#include "Serialization/BufferArchive.h"
#include "RPCPacketBase.generated.h"

enum class ERPCPacketTypes : uint16;

USTRUCT()
struct FRPCPacketBase
{
	GENERATED_BODY()
public:
	UPROPERTY()
	FGuid SerialNumber; ///Todo - 패킷을 구분

	UPROPERTY()
	uint64 TimeStamp = 0; ///Todo - UTF_0 기준 표준시. 

	UPROPERTY()
	int32 Version = 0; ///Todo - 추후 업데이트시 패킷 구조가 맞지 않는 경우를 대비해서

public:
	virtual ~FRPCPacketBase() = default;

	// 공통 직렬화 로직 (Overridable)
	virtual void SerializePacket(FArchive& inArchive)
	{
		inArchive << SerialNumber;
		inArchive << TimeStamp;
		inArchive << Version;
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


USTRUCT()
struct FRPCPacketC2S_OneParam_Int : public FRPCPacket_C2S
{
	GENERATED_BODY()
public:
	UPROPERTY()
	int32 value = 0;

public:
	virtual void SerializePacket(FArchive& inArchive) override
	{
		FRPCPacket_C2S::SerializePacket(inArchive);
		inArchive << value;
	}
};

// 테스트 패킷
USTRUCT()
struct FRPCPacketS2C_OneParam_Int : public FRPCPacket_S2C
{
	GENERATED_BODY()
public:
	UPROPERTY()
	int32 value = 0;

public:
	virtual void SerializePacket(FArchive& inArchive) override
	{
		FRPCPacket_S2C::SerializePacket(inArchive);
		inArchive << value;
	}
};

USTRUCT()
struct FRPCPacketS2C_Error : public FRPCPacket_S2C
{
	GENERATED_BODY()
public:
	///Todo : 응답받고 큐에서 제거하고... 흠....
	   // 실패한 원본 요청의 패킷 타입
	UPROPERTY()
	ERPCPacketTypes OriginalRequestType;

	UPROPERTY()
	int32 ErrorCode;

	// (선택 사항) 좀 더 상세한 에러 메시지 문자열
	UPROPERTY()
	FString ErrorMessage;

public:
	virtual void SerializePacket(FArchive& inArchive) override
	{
		FRPCPacket_S2C::SerializePacket(inArchive); // 부모 클래스 직렬화 (SerialNumber, TimeStamp, ResponseCode 처리)
		inArchive << OriginalRequestType;
		inArchive << ErrorCode;
		inArchive << ErrorMessage;
	}
};
