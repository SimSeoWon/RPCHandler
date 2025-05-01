// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ServerTestCharacter.generated.h"

UCLASS(Blueprintable)
class AServerTestCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AServerTestCharacter();

	// Called every frame.
	virtual void Tick(float DeltaSeconds) override;
	virtual void PostNetInit() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_Owner() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnActorChannelOpen(class FInBunch& InBunch, class UNetConnection* Connection) override;
	virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const override;
	/** Returns TopDownCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;
public:
	//UPROPERTY(Replicated)
	UPROPERTY(ReplicatedUsing = OnRep_ServerRotationYaw)
	float ServerRotationYaw;

	UFUNCTION()
	void OnRep_ServerRotationYaw();

	UFUNCTION()
	void OnRep_ServerColor();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPC_ChangeColor(const FLinearColor& inColor); // 서버가 모든 클라이언트에게 색상을 변경하라고 요청한다.


	UFUNCTION(Server, Unreliable, WithValidation)
	void ServerRPC_ChangeColor(); // 클라이언트가 서버에게 

	UFUNCTION(Client, Unreliable)
	void ClientRPC_ChangeColor(const FLinearColor& inColor); // 서버가 클라이언트에게 색상을 변경하라고 요청한다.



private:
	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	

	//UPROPERTY(Replicated)
	//TArray<float> BigData;

	UPROPERTY(ReplicatedUsing = OnRep_ServerColor)
	FLinearColor ServerColor;

	

	float RotationRate = 30.0f;
	float ClientTimeSceneUpdate = 0.0f;
	float ClientTimeBetweenLastUpdate = 0.0f;
	//float BigDataElement = 0.0f;

	FTimerHandle Handle;
};

