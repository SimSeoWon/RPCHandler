// Copyright Epic Games, Inc. All Rights Reserved.

#include "ServerTestPlayerController.h"
#include "GameFramework/Pawn.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "ServerTestCharacter.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "RPCHandler/RPCHandlerComponent.h"

#include "ServerTest.h"
DEFINE_LOG_CATEGORY(LogTemplateCharacter);

AServerTestPlayerController::AServerTestPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
	CachedDestination = FVector::ZeroVector;
	FollowTime = 0.f;

	RPCHandler = CreateDefaultSubobject<URPCHandlerComponent>(TEXT("RPCHandler"));	
	RPCHandler->SetIsReplicated(true);
}

void AServerTestPlayerController::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	TEST_LOG(LogServerTest, Log, TEXT("%s"), TEXT("Begin"));
	if (HasAuthority())
	{
		
	}
}
void AServerTestPlayerController::PostInitializeComponents()
{
	TEST_LOG(LogServerTest, Log, TEXT("%s"), TEXT("Begin"));
	Super::PostInitializeComponents();
	TEST_LOG(LogServerTest, Log, TEXT("%s"), TEXT("End"));
}

void AServerTestPlayerController::PostNetInit()
{
	TEST_LOG(LogServerTest, Log, TEXT("%s"), TEXT("Begin"));
	Super::PostNetInit();

	UNetDriver* netDriver = GetNetDriver();
	if (IsValid(netDriver))
	{
		if (IsValid(netDriver->ServerConnection))
		{
			TEST_LOG(LogServerTest, Log, TEXT("Server Connection - %s"), *netDriver->ServerConnection->GetName());
		}
		else
		{
			TEST_LOG(LogServerTest, Log, TEXT("%s"), TEXT("Server Connection is not valid"));
		}	
	}
	else
	{
		TEST_LOG(LogServerTest, Log, TEXT("%s"), TEXT("netDriver is not valid"));
	}

	TEST_LOG(LogServerTest, Log, TEXT("%s"), TEXT("End"));

}

void AServerTestPlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

	// Add Input Mapping Context
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		// Setup mouse input events
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Started, this, &AServerTestPlayerController::OnInputStarted);
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Triggered, this, &AServerTestPlayerController::OnSetDestinationTriggered);
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Completed, this, &AServerTestPlayerController::OnSetDestinationReleased);
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Canceled, this, &AServerTestPlayerController::OnSetDestinationReleased);

		// Setup touch input events
		EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Started, this, &AServerTestPlayerController::OnInputStarted);
		EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Triggered, this, &AServerTestPlayerController::OnTouchTriggered);
		EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Completed, this, &AServerTestPlayerController::OnTouchReleased);
		EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Canceled, this, &AServerTestPlayerController::OnTouchReleased);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AServerTestPlayerController::OnInputStarted()
{
	StopMovement();
}

// Triggered every frame when the input is held down
void AServerTestPlayerController::OnSetDestinationTriggered()
{
	// We flag that the input is being pressed
	FollowTime += GetWorld()->GetDeltaSeconds();

	// We look for the location in the world where the player has pressed the input
	FHitResult Hit;
	bool bHitSuccessful = false;
	if (bIsTouch)
	{
		bHitSuccessful = GetHitResultUnderFinger(ETouchIndex::Touch1, ECollisionChannel::ECC_Visibility, true, Hit);
	}
	else
	{
		bHitSuccessful = GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, Hit);
	}

	// If we hit a surface, cache the location
	if (bHitSuccessful)
	{
		CachedDestination = Hit.Location;
	}

	// Move towards mouse pointer or touch
	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn != nullptr)
	{
		FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
		ControlledPawn->AddMovementInput(WorldDirection, 1.0, false);
	}
}

void AServerTestPlayerController::OnSetDestinationReleased()
{
	// If it was a short press
	if (FollowTime <= ShortPressThreshold)
	{
		// We move there and spawn some particles
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, CachedDestination);
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, CachedDestination, FRotator::ZeroRotator, FVector(1.f, 1.f, 1.f), true, true, ENCPoolMethod::None, true);
	}

	FollowTime = 0.f;
}

// Triggered every frame when the input is held down
void AServerTestPlayerController::OnTouchTriggered()
{
	bIsTouch = true;
	OnSetDestinationTriggered();
}

void AServerTestPlayerController::OnTouchReleased()
{
	bIsTouch = false;
	OnSetDestinationReleased();
}

void AServerTestPlayerController::OnPossess(APawn* aPawn)
{
	TEST_LOG(LogServerTest, Log, TEXT("%s"), TEXT("Begin"));
	Super::OnPossess(aPawn);
	TEST_LOG(LogServerTest, Log, TEXT("%s"), TEXT("End"));
}
