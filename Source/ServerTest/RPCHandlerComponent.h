// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Serialization/BufferArchive.h"
#include "RPCHandlerComponent.generated.h"

class AServerTestPlayerController;

namespace RPCHandler
{
	// TFunction<void(const FRPCPacketWrapper&)> Ÿ�Կ� ���� ª�� �̸� ����
	using HandlerDelegate = TFunction<void(const FRPCPacketWrapper&)>;
}

UENUM()
enum class EPacketType : uint16
{
	NONE,
	ChangeColor,
	ChangeColor_Response,
	MAX,
};

USTRUCT()
struct FRPCPacketBase
{
	GENERATED_BODY()
public:
	UPROPERTY()
	FGuid SerialNumber; // ��Ŷ ������

	UPROPERTY()
	uint64 TimeStamp = 0; //UTF-0 ���� ǥ�ؽð�. 

public:
	// ���� ����ȭ ���� (Overridable)
	virtual void SerializePacket(FArchive& inArchive)
	{
		inArchive << SerialNumber;
		inArchive << TimeStamp;
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

// �׽�Ʈ ��Ŷ
USTRUCT()
struct FRPCPacketReq_ChangeColor : public FRPCPacket_C2S
{
	GENERATED_BODY()
public:
	UPROPERTY()
	FLinearColor Color = FLinearColor::White; // ������ �÷� ����.

public:
	virtual void SerializePacket(FArchive& inArchive) override
	{
		FRPCPacket_C2S::SerializePacket(inArchive);
		inArchive << Color;
	}
};

// �׽�Ʈ ��Ŷ
USTRUCT()
struct FRPCPacketRes_ChangeColor : public FRPCPacket_S2C
{
	GENERATED_BODY()
public:
	///Todo : ����ް� ť���� �����ϰ�... ��....

public:
	virtual void SerializePacket(FArchive& inArchive) override
	{
		FRPCPacket_S2C::SerializePacket(inArchive);
		
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

public:
	// Ŭ���̾�Ʈ�� ������ ��Ŷ�� ���� �� ȣ��
	UFUNCTION(Server, Reliable, WithValidation) // �������� ���� + ���� �Լ� �ʿ�
		void Server_SendPacket(FRPCPacketWrapper PacketWrapper);
	bool Server_SendPacket_Validate(FRPCPacketWrapper PacketWrapper);
	void Server_SendPacket_Implementation(FRPCPacketWrapper PacketWrapper);

	// ������ Ŭ���̾�Ʈ�� ��Ŷ�� ���� �� ȣ��
	UFUNCTION(Client, Reliable)
	void Client_ReceivePacket(FRPCPacketWrapper PacketWrapper);
	void Client_ReceivePacket_Implementation(FRPCPacketWrapper PacketWrapper);

	UFUNCTION()
	void OnReq_ChangeColor(FRPCPacketWrapper inPacketWrapper);

protected:
	AServerTestPlayerController* FindPlayerControllerById(int32 inPlayerId);

	// �ɼ� A: ��� �Լ� ������ ���
	TMap<EPacketType, RPCHandler::HandlerDelegate> DelegateFunctionMap;

};
