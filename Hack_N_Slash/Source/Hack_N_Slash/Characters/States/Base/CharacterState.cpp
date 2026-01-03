#include "CharacterState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

/*--------------------------------- UCharacterState ---------------------------------*/
void UCharacterState::Initialize(UStateMachineComponent* InSM, ACharacter* InOwner)
{
    ownerStateMachineComp = InSM;
    ownerChar = InOwner;
}

bool UCharacterState::CanBeInterruptedBy(const UCharacterState* Other) const
{
    if (!Other) return false;
    return Other->GetPriority() >= GetPriority();
}

/*--------------------------------- UMovementState ---------------------------------*/
//Lifecycle
void UMovementState::EnterState()
{
    MoveComp = ownerChar ? ownerChar->GetCharacterMovement() : nullptr;

    if (!ownerChar || !MoveComp) return;

    // Bind movement events (no tick needed)
    ownerChar->LandedDelegate.AddDynamic(this, &UMovementState::OnLanded);
    ownerChar->MovementModeChangedDelegate.AddDynamic(this, &UMovementState::OnMovementModeChanged);

    // Initialize grounded timestamp
    if (!IsFalling()) LastGroundedTime = ownerChar->GetWorld()->GetTimeSeconds();

    // Choose starting baseline substate
    EvaluateBaselineSubState();
}

void UMovementState::ExitState()
{
    if (ownerChar)
    {
        ownerChar->MovementModeChangedDelegate.RemoveDynamic(this, &UMovementState::OnMovementModeChanged);
        ownerChar->LandedDelegate.RemoveDynamic(this, &UMovementState::OnLanded);

        // Clear timers
        if (UWorld* World = ownerChar->GetWorld()) World->GetTimerManager().ClearTimer(JumpBufferTimerHandle);
    }

    if (ActiveSubState)
    {
        ActiveSubState->ExitState();
        ActiveSubState = nullptr;
    }
}

//Input/Event Handlers
void UMovementState::OnInputMove(const FVector2D& Move)
{
    InputCtx.move = Move;
    if (ActiveSubState) ActiveSubState->OnInputMove(Move);
}

void UMovementState::OnInputLook(const FVector2D& Look)
{
    InputCtx.look = Look;
    if (ActiveSubState) ActiveSubState->OnInputLook(Look);
}

void UMovementState::OnInputJumpPressed()
{
    if (!ownerChar) return;
    InputCtx.bWantsJump = true;
    InputCtx.jumpPressedTime = ownerChar->GetWorld()->GetTimeSeconds();

    StartJumpBufferWindow();
    TryConsumeBufferedJump();

    if (ActiveSubState) ActiveSubState->OnInputJumpPressed();
}

void UMovementState::OnInputJumpReleased()
{
    // Optional: variable jump height support lives in Airborne/Jump substate typically.
}

void UMovementState::OnLanded(const FHitResult& Hit)
{
    if (!ownerChar) return;

    LastGroundedTime = ownerChar->GetWorld()->GetTimeSeconds();
    // Landed is a great moment to swap to grounded or a dedicated LandState later
    EvaluateBaselineSubState();

    // Consume buffered jump on landing if still valid
    TryConsumeBufferedJump();

    if (ActiveSubState) ActiveSubState->OnLanded(Hit);
}

void UMovementState::OnMovementModeChanged(ACharacter* InCharacter, EMovementMode PrevMovementMode, uint8 PrevCustomMode)
{
    if (!ownerChar || !MoveComp) return;

    // Update grounded time when we enter walking/navwalking
    if (!IsFalling())
    {
        LastGroundedTime = ownerChar->GetWorld()->GetTimeSeconds();

        // If we buffered jump slightly before landing, consume now
        TryConsumeBufferedJump();
    }

    // Baseline substate swap grounded/airborne
    EvaluateBaselineSubState();

    if (ActiveSubState) ActiveSubState->OnMovementModeChanged(InCharacter, PrevMovementMode, PrevCustomMode);
}

//Helpers
bool UMovementState::IsFalling() const
{
    return MoveComp && MoveComp->IsFalling();
}

void UMovementState::EvaluateBaselineSubState()
{
    if (!MoveComp) return;

    if (IsFalling())
    {
        if (DefaultAirborneStateClass) SetSubState(DefaultAirborneStateClass);
    }
    else
    {
        if (DefaultGroundedStateClass) SetSubState(DefaultGroundedStateClass);
    }
}

void UMovementState::SetSubState(TSubclassOf<UMovementState> NewSubStateClass)
{
    if (!ownerStateMachineComp || !NewSubStateClass) return;

    if (ActiveSubState && ActiveSubState->IsA(NewSubStateClass)) return;

    if (ActiveSubState)
    {
        ActiveSubState->ExitState();
        ActiveSubState = nullptr;
    }

    //ActiveSubState = ownerStateMachineComp->GetOrCreateStateInstance(NewSubStateClass);
    if (ActiveSubState)
    {
        ActiveSubState->Initialize(ownerStateMachineComp, ownerChar);
        ActiveSubState->EnterState();
    }
}

void UMovementState::StartJumpBufferWindow()
{
    if (!ownerChar) return;

    UWorld* World = ownerChar->GetWorld();
    if (!World) return;

    World->GetTimerManager().ClearTimer(JumpBufferTimerHandle);
    World->GetTimerManager().SetTimer(
        JumpBufferTimerHandle,
        this,
        &UMovementState::ExpireJumpBuffer,
        JumpBufferSeconds,
        false
    );
}

void UMovementState::ExpireJumpBuffer()
{
    InputCtx.ClearJump();
}

bool UMovementState::CanUseBufferedJump() const
{
    if (!ownerChar) return false;

    const float Now = ownerChar->GetWorld()->GetTimeSeconds();
    const bool bBuffered = InputCtx.bWantsJump && (Now - InputCtx.jumpPressedTime) <= JumpBufferSeconds;
    const bool bGroundOrCoyote = !IsFalling() || ((Now - LastGroundedTime) <= CoyoteSeconds);

    return bBuffered && bGroundOrCoyote;
}

void UMovementState::TryConsumeBufferedJump()
{
    if (!ownerChar) return;

    if (CanUseBufferedJump())
    {
        // IMPORTANT: Don’t actually call Char->Jump() here if you want JumpState to own it.
        // Instead, transition to a Jump substate, and let JumpState do Char->Jump() in EnterState().

        // Example (once you make it):
        // SetSubState(JumpStateClass);

        // For now, just clear the request so it doesn't repeat.
        InputCtx.ClearJump();
    }
}