#include "CharacterState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "../../StateMachineComponent.h"

/*--------------------------------- UCharacterState ---------------------------------*/
void UCharacterState::Initialize(UStateMachineComponent* InSM, ACharacter* InOwner)
{
    if (bInitialized) return;
    bInitialized = true;
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
    moveComp = ownerChar ? ownerChar->GetCharacterMovement() : nullptr;

    if (!ownerChar || !moveComp) return;

    // Bind movement events (no tick needed)
    ownerChar->LandedDelegate.AddDynamic(this, &UMovementState::OnLanded);
    ownerChar->MovementModeChangedDelegate.AddDynamic(this, &UMovementState::OnMovementModeChanged);

    // Initialize grounded timestamp
    if (!IsFalling()) lastGroundedTime = ownerChar->GetWorld()->GetTimeSeconds();
}

void UMovementState::ExitState()
{
    if (ownerChar)
    {
        ownerChar->MovementModeChangedDelegate.RemoveDynamic(this, &UMovementState::OnMovementModeChanged);
        ownerChar->LandedDelegate.RemoveDynamic(this, &UMovementState::OnLanded);

        // Clear timers
        if (UWorld* World = ownerChar->GetWorld()) World->GetTimerManager().ClearTimer(TH_JumpBuffer);
    }

    if (activeSubState)
    {
        activeSubState->ExitState();
        activeSubState = nullptr;
    }
}

//Input/Event Handlers
bool UMovementState::OnInputMove(const FVector2D& Move)
{
    inputCtx.move = Move;
    return activeSubState ? activeSubState->OnInputMove(Move) : false;// Movement usually returns false, but special substates can return true.
}

bool UMovementState::OnInputLook(const FVector2D& Look)
{
    inputCtx.look = Look;
    return activeSubState ? activeSubState->OnInputLook(Look) : false;
}

bool UMovementState::OnInputJumpPressed()
{
    if (!ownerChar) return false;
    inputCtx.bWantsJump = true;
    inputCtx.jumpPressedTime = ownerChar->GetWorld()->GetTimeSeconds();

    StartJumpBufferWindow();
    TryConsumeBufferedJump();

    return activeSubState ? activeSubState->OnInputJumpPressed() : false;
}

bool UMovementState::OnInputJumpReleased()
{
    // Optional: variable jump height support lives in Airborne/Jump substate typically.
    return activeSubState ? activeSubState->OnInputJumpReleased() : false;
}

void UMovementState::OnLanded(const FHitResult& Hit)
{
    if (!ownerChar) return;

    lastGroundedTime = ownerChar->GetWorld()->GetTimeSeconds();

    // Root-only baseline swap
    ApplyBaselineSubState();

    // Consume buffered jump on landing if still valid
    TryConsumeBufferedJump();

    if (activeSubState) activeSubState->OnLanded(Hit);
}

void UMovementState::OnMovementModeChanged(ACharacter* InCharacter, EMovementMode PrevMovementMode, uint8 PrevCustomMode)
{
    if (!ownerChar || !moveComp) return;

    // Update grounded time when we enter walking/navwalking
    if (!IsFalling())
    {
        lastGroundedTime = ownerChar->GetWorld()->GetTimeSeconds();

        // If we buffered jump slightly before landing, consume now
        TryConsumeBufferedJump();
    }

    // Root-only baseline swap
    ApplyBaselineSubState();

    if (activeSubState) activeSubState->OnMovementModeChanged(InCharacter, PrevMovementMode, PrevCustomMode);
}

//Helpers
bool UMovementState::IsFalling() const
{
    return moveComp && moveComp->IsFalling();
}

void UMovementState::EvaluateBaselineSubState()
{
    if (!moveComp) return;

    if (IsFalling())
    {
        if (defaultAirborneStateClass) SetSubState(defaultAirborneStateClass);
    }
    else
    {
        if (defaultGroundedStateClass) SetSubState(defaultGroundedStateClass);
    }
}

void UMovementState::SetSubState(TSubclassOf<UMovementState> NewSubStateClass)
{
    if (!ownerStateMachineComp) return;

    UClass* DesiredClass = NewSubStateClass.Get();
    if (!DesiredClass) return;

    // Never allow abstract classes as actual active substates
    if (DesiredClass->HasAnyClassFlags(CLASS_Abstract))
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] SetSubState rejected: %s is abstract."), *GetNameSafe(this), *GetNameSafe(DesiredClass));
        return;
    }

    // Structural guard: Don't set a state as its own substate by class
    if (DesiredClass == GetClass())
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] SetSubState rejected: cannot set substate to self class (%s)."), *GetNameSafe(this), *GetNameSafe(DesiredClass));
        return;
    }

    // Idempotency: If we're already in the exact desired class, do nothing
    if (activeSubState && activeSubState->GetClass() == DesiredClass) return;

    // Resolve the instance (must return a concrete instance, not null)
    UMovementState* NewState = ownerStateMachineComp->GetMovementState(NewSubStateClass);

    // Guard: Resolution failed (not registered / couldn't be created)
    if (!NewState)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] SetSubState failed: no instance found for %s."), *GetNameSafe(this), *GetNameSafe(DesiredClass));
        return;
    }

    // Structural guard: Never set substate instance to self instance
    if (NewState == this)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] SetSubState rejected: cannot set substate to self instance (%s)."), *GetNameSafe(this), *GetNameSafe(DesiredClass));
        return;
    }

    //Prevents unwanted state transitions
    //EX: Don’t allow grind if not near rail
    if (!NewState->CanEnterState(this)) {return;}

    // Transition: exit old, enter new
    if (activeSubState) activeSubState->ExitState();

    activeSubState = NewState;

    // IMPORTANT: Initialize should be idempotent (only does work once)
    // If yours isn't, make it so (bInitialized guard inside Initialize)
    activeSubState->Initialize(ownerStateMachineComp, ownerChar);
    activeSubState->EnterState();
}


void UMovementState::StartJumpBufferWindow()
{
    if (!ownerChar) return;

    UWorld* World = ownerChar->GetWorld();
    if (!World) return;

    World->GetTimerManager().ClearTimer(TH_JumpBuffer);
    World->GetTimerManager().SetTimer(
        TH_JumpBuffer,
        this,
        &UMovementState::ExpireJumpBuffer,
        jumpBufferSeconds,
        false
    );
}

void UMovementState::ExpireJumpBuffer()
{
    inputCtx.ClearJump();
}

bool UMovementState::CanUseBufferedJump() const
{
    if (!ownerChar) return false;

    const float Now = ownerChar->GetWorld()->GetTimeSeconds();
    const bool bBuffered = inputCtx.bWantsJump && (Now - inputCtx.jumpPressedTime) <= jumpBufferSeconds;
    const bool bGroundOrCoyote = !IsFalling() || ((Now - lastGroundedTime) <= coyoteSeconds);

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
        inputCtx.ClearJump();
    }
}
/*--------------------------------- UActionState ---------------------------------*/

bool UActionState::OnInputJumpPressed()
{
    return false;
}

bool UActionState::OnInputJumpReleased()
{
    return false;
}

bool UActionState::OnInputLook(const FVector2D &InputVector)
{
    return false;
}

bool UActionState::OnInputMove(const FVector2D &Move)
{
    return false;
}