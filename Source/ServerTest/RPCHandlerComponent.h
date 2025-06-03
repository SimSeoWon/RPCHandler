#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Handlers/RPCPacketWrapper.h"
#include "RPCHandlerComponent.generated.h"

class AServerTestPlayerController;
class IRPCFunctor;

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
	//void SavedCheaterInfo(const FString& inReason, FRPCPacketWrapper inWrapper);
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
	bool Client_ReceivePacket_Validate(FRPCPacketWrapper PacketWrapper);
	void Client_ReceivePacket_Implementation(FRPCPacketWrapper PacketWrapper);
#pragma endregion  RPC core

#pragma region server side processing
protected:
	TMap<ERPCPacketTypes, TSharedPtr<IRPCFunctor>> FunctorMap;
	TArray<TSharedPtr<IRPCFunctor>> FunctorList;
#pragma endregion  server side processing

protected:
	AServerTestPlayerController* FindPlayerControllerById(int32 inPlayerId);
};
