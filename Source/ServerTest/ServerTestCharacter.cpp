// Copyright Epic Games, Inc. All Rights Reserved.

#include "ServerTestCharacter.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Materials/Material.h"
#include "Engine/World.h"
#include "EngineUtils.h"

#include "ServerTestPlayerController.h"
#include "RPCRequestManager.h"
#include "ServerTest.h"

AServerTestCharacter::AServerTestCharacter()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to moving direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true); // Don't want arm to rotate when character does
	CameraBoom->TargetArmLength = 800.f;
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a camera...
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// ����ȭ �� ���̴�.
	bReplicates = true;
	NetUpdateFrequency = 1.0f;
	//NetDormancy = DORM_Initial; // �޸� ����
}
void AServerTestCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateLambda([this]()
		{
			URPCRequestManager* manager = URPCRequestManager::Get(this, 0);
			if (false == IsValid(manager))
				return;

			manager->ReqChangeColor();

		}), 1.0f, true, 0.0f);

	/*if (false == HasAuthority())
		return;*/
}

void AServerTestCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearTimer(Handle);

	Super::EndPlay(EndPlayReason);
}

void AServerTestCharacter::OnActorChannelOpen(class FInBunch& InBunch, class UNetConnection* Connection)
{
	TEST_LOG(LogServerTest, Log, TEXT("%s"), TEXT("Begin"));

	Super::OnActorChannelOpen(InBunch, Connection);

	TEST_LOG(LogServerTest, Log, TEXT("%s"), TEXT("End"));
}

bool AServerTestCharacter::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
	TEST_LOG(LogServerTest, Log, TEXT("%s"), TEXT("Begin"));
	return Super::IsNetRelevantFor(RealViewer, ViewTarget, SrcLocation);
	//TEST_LOG(LogServerTest, Log, TEXT("%s"), TEXT("End"));
}

void AServerTestCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AServerTestCharacter, ServerRotationYaw);
	//DOREPLIFETIME(AServerTestCharacter, BigData);
	//DOREPLIFETIME_CONDITION(AServerTestCharacter, ServerColor, COND_InitialOnly); //���ʿ� 1���� ������.
	DOREPLIFETIME(AServerTestCharacter, ServerColor); //���ʿ� 1���� ������.
}

void AServerTestCharacter::OnRep_ServerRotationYaw()
{
	//TEST_LOG(LogServerTest, Log, TEXT("%s"), TEXT("Begin"));
	FRotator newRotater = RootComponent->GetComponentRotation();
	newRotater.Yaw = ServerRotationYaw;
	RootComponent->SetWorldRotation(newRotater);
	//TEST_LOG(LogServerTest, Log, TEXT("%s"), TEXT("End"));
}

void AServerTestCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (false == HasAuthority())
	{
		ClientTimeSceneUpdate += DeltaSeconds;

		if (ClientTimeSceneUpdate < KINDA_SMALL_NUMBER)
			return;

		const float EstimateRotationYaw = ServerRotationYaw + (RotationRate * ClientTimeBetweenLastUpdate);
		const float LerpRatio = ClientTimeSceneUpdate / ClientTimeBetweenLastUpdate;

		FRotator ClientRotator = RootComponent->GetComponentRotation();
		const float ClientNewYaw = FMath::Lerp(ServerRotationYaw, EstimateRotationYaw, LerpRatio);
		ClientRotator.Yaw = ClientNewYaw;
		RootComponent->SetWorldRotation(ClientRotator);
		return;
	}


	//ServerRotationYaw = GetActorRotation().Yaw;
	AddActorLocalRotation(FRotator(0.f, RotationRate * DeltaSeconds, 0.f));
	ServerRotationYaw = RootComponent->GetComponentRotation().Yaw;
}

void AServerTestCharacter::PossessedBy(AController* NewController)
{
	TEST_LOG(LogServerTest, Log, TEXT("%s"), TEXT("Begin"));
	AActor* owner = GetOwner();
	if (IsValid(owner))
	{
		TEST_LOG(LogServerTest, Log, TEXT("Owner - %s"), *owner->GetName());
	}
	else
	{
		TEST_LOG(LogServerTest, Log, TEXT("%s"), TEXT("Owner is not valid"));
	}

	Super::PossessedBy(NewController);

	owner = GetOwner();
	if (IsValid(owner))
	{
		TEST_LOG(LogServerTest, Log, TEXT("Owner - %s"), *owner->GetName());
	}
	else
	{
		TEST_LOG(LogServerTest, Log, TEXT("%s"), TEXT("Owner is not valid"));
	}

	TEST_LOG(LogServerTest, Log, TEXT("%s"), TEXT("End"));
}

void AServerTestCharacter::PostNetInit()
{
	TEST_LOG(LogServerTest, Log, TEXT("%s %s"), TEXT("Begin"), *GetName());
	Super::PostNetInit();
	TEST_LOG(LogServerTest, Log, TEXT("%s %s"), TEXT("End"), *GetName());
}

void AServerTestCharacter::OnRep_Owner()
{
	TEST_LOG(LogServerTest, Log, TEXT("%s"), TEXT("Begin"));
	Super::OnRep_Owner();
	AActor* owner = GetOwner();
	if (IsValid(owner))
	{
		TEST_LOG(LogServerTest, Log, TEXT("Owner - %s"), *owner->GetName());
	}
	else
	{
		TEST_LOG(LogServerTest, Log, TEXT("%s"), TEXT("Owner is not valid"));
	}
	TEST_LOG(LogServerTest, Log, TEXT("%s"), TEXT("End"));

	ClientTimeBetweenLastUpdate = ClientTimeSceneUpdate;
	ClientTimeSceneUpdate = 0.0f;;
}

void AServerTestCharacter::OnRep_ServerColor()
{
	TEST_LOG(LogServerTest, Log, TEXT("%s"), TEXT("Begin"));

	TEST_LOG(LogServerTest, Log, TEXT("Owner - %s"), *ServerColor.ToString());

	TEST_LOG(LogServerTest, Log, TEXT("%s"), TEXT("End"));
}

void AServerTestCharacter::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	TEST_LOG(LogServerTest, Log, TEXT("%s"), TEXT("Begin"));
	Super::PreReplication(ChangedPropertyTracker);
	TEST_LOG(LogServerTest, Log, TEXT("%s"), TEXT("End"));
}

void AServerTestCharacter::MulticastRPC_ChangeColor_Implementation(const FLinearColor& inColor)
{
	TEST_LOG(LogServerTest, Log, TEXT("%s"), TEXT("Begin"));

	ServerColor = inColor;
	TEST_LOG(LogServerTest, Log, TEXT("Owner - %s"), *ServerColor.ToString());

	TEST_LOG(LogServerTest, Log, TEXT("%s"), TEXT("End"));
}

void AServerTestCharacter::ServerRPC_ChangeColor_Implementation()
{
	const FLinearColor newColor = FLinearColor(FMath::RandRange(0.0f, 1.0f), FMath::RandRange(0.0f, 1.0f), FMath::RandRange(0.0f, 1.0f), 1.0f);
	//MulticastRPC_ChangeColor(newColor);
	ClientRPC_ChangeColor(newColor); // Ŭ���̾�Ʈ�� ���ʽ��� ���ٸ� �������� �۵��ϰ�, ������ �Ǹ� Ŭ���̾�Ʈ���� �۵���.
}

bool AServerTestCharacter::ServerRPC_ChangeColor_Validate()
{
	TEST_LOG(LogServerTest, Log, TEXT("%s"), TEXT("Begin"));
	//��ȿ�� �˻�. ���н� ������ ������ ������.
	return true;
}


void AServerTestCharacter::ClientRPC_ChangeColor_Implementation(const FLinearColor& inColor) 
{
	TEST_LOG(LogServerTest, Log, TEXT("%s"), TEXT("Begin"));

	ServerColor = inColor;
	TEST_LOG(LogServerTest, Log, TEXT("Owner - %s"), *ServerColor.ToString());

	TEST_LOG(LogServerTest, Log, TEXT("%s"), TEXT("End"));

}

