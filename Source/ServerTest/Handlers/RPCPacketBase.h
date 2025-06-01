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
	FGuid SerialNumber; ///Todo - ��Ŷ�� ����

	UPROPERTY()
	uint64 TimeStamp = 0; ///Todo - UTF_0 ���� ǥ�ؽ�. 

	UPROPERTY()
	int32 Version = 0; ///Todo - ���� ������Ʈ�� ��Ŷ ������ ���� �ʴ� ��츦 ����ؼ�

public:
	virtual ~FRPCPacketBase() = default;

	// ���� ����ȭ ���� (Overridable)
	virtual void SerializePacket(FArchive& inArchive)
	{
		inArchive << SerialNumber;
		inArchive << TimeStamp;
		inArchive << Version;
	}
};

USTRUCT()
struct FRPCPacket_C2S : public FRPCPacketBase ///Todo - Ŭ���̾�Ʈ�� ������ ��û��.
{
	GENERATED_BODY()
public:

	UPROPERTY()
	int32 PlayerNumber = -1; // �÷��̾� ���̵�
	

	virtual void SerializePacket(FArchive& inArchive) override
	{
		FRPCPacketBase::SerializePacket(inArchive);
		inArchive << PlayerNumber;
	}
};

USTRUCT()
struct FRPCPacket_S2C : public FRPCPacketBase ///Todo - ������ Ŭ���̾�Ʈ���� ������. 
{
	 GENERATED_BODY()
public:

	UPROPERTY()
	int32 ResponseCode = -1; // ����, ����, ���� �ڵ� ��...
	
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

// �׽�Ʈ ��Ŷ
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
	///Todo : ����ް� ť���� �����ϰ�... ��....
	   // ������ ���� ��û�� ��Ŷ Ÿ��
	UPROPERTY()
	ERPCPacketTypes OriginalRequestType;

	UPROPERTY()
	int32 ErrorCode;

	// (���� ����) �� �� ���� ���� �޽��� ���ڿ�
	UPROPERTY()
	FString ErrorMessage;

public:
	virtual void SerializePacket(FArchive& inArchive) override
	{
		FRPCPacket_S2C::SerializePacket(inArchive); // �θ� Ŭ���� ����ȭ (SerialNumber, TimeStamp, ResponseCode ó��)
		inArchive << OriginalRequestType;
		inArchive << ErrorCode;
		inArchive << ErrorMessage;
	}
};
