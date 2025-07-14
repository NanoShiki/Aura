// Copyright Nanoshiki


#include "Player/AuraPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Interaction/EnemyInterface.h"

AAuraPlayerController::AAuraPlayerController() {
	bReplicates = true;
}

void AAuraPlayerController::PlayerTick(float DeltaTime) {
	Super::PlayerTick(DeltaTime);

	CursorTrace();
}

void AAuraPlayerController::CursorTrace() {
	FHitResult CursorHit;
	GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);
	if (!CursorHit.bBlockingHit) return;

	LastActor = ThisActor;
	ThisActor = Cast<IEnemyInterface>(CursorHit.GetActor());

	/**
	 * Line trace from cursor. There are several scenarios:
	 *  A. LastActor is null && ThisActor is null
	 *		- Do nothing
	 *	B. LastActor is null && ThisActor is valid
	 *		- HighLight ThisActor
	 *	C. LastActor is valid && ThisActor is null
	 *		- UnHighLight LastActor
	 *	D. Both actors are valid, but LastActor != ThisActor
	 *		- UnHighLight LastActor, and HighLight ThisActor
	 *	E. Both actors are valid, and are the same actor
	 *		- Do nothing
	 */
	if (LastActor == nullptr) {
		if (ThisActor != nullptr) {
			// Case B
			ThisActor->HighLightActor();
		}
		else {
			// Case A
		}
	}
	else {
		if (ThisActor == nullptr) {
			// Case C
			LastActor->UnHighLightActor();
		}
		else {
			if (LastActor != ThisActor) {
				// Case D
				LastActor->UnHighLightActor();
				ThisActor->HighLightActor();
			}
			else {
				// Case E
			}
		}
	}
}

void AAuraPlayerController::BeginPlay() {
	Super::BeginPlay();
	check(AuraContext);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Subsystem) {
		Subsystem->AddMappingContext(AuraContext, 0);
	}
	

	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;

	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);
	SetInputMode(InputModeData);
}

void AAuraPlayerController::SetupInputComponent() {
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);

	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
}

void AAuraPlayerController::Move(const FInputActionValue& InputActionValue) {
	/** Get Forward and Right Direction */
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
	const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	/** Add Movement Input According to the InputActionValue */
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
	if (APawn* ControlledPawn = GetPawn<APawn>()) {
		ControlledPawn->AddMovementInput(Forward, InputAxisVector.Y);
		ControlledPawn->AddMovementInput(Right, InputAxisVector.X);
	}
}



