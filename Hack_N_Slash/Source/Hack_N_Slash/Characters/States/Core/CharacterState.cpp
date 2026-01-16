#include "CharacterState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../../StateMachineComponent.h" // for FCommandContext + interface access if needed

/*--------------------------------- UCharacterState ---------------------------------*/

void UCharacterState::Initialize(UStateMachineComponent* InSM, ACharacter* InOwner)
{
    if (bInitialized) return;

    ownerStateMachineComp = InSM;
    ownerChar = InOwner;

    if (ownerStateMachineComp && ownerChar) bInitialized = true;
    else UE_LOG(LogTemp, Warning, TEXT("[%s] Initialization failed. StateMachineComp and/or Character is null"), *GetNameSafe(this));
}

bool UCharacterState::CanBeInterruptedBy(const UCharacterState* Other) const
{
    if (!Other) return false;
    return Other->GetPriority() >= GetPriority();
}

/*--------------------------------- UMovementState ---------------------------------*/

void UMovementState::Initialize(UStateMachineComponent* InSM, ACharacter* InOwner)
{
    Super::Initialize(InSM, InOwner);
    moveComp = ownerChar ? ownerChar->GetCharacterMovement() : nullptr;
}

void UMovementState::EnterState()
{
    Super::EnterState();

    // Initialize grounded time if we enter while grounded
    if (ownerChar && moveComp && moveComp->IsMovingOnGround()) MarkGroundedNow();
}

void UMovementState::ExitState()
{
    if (ownerChar)
    {
        if (UWorld* World = ownerChar->GetWorld()) World->GetTimerManager().ClearTimer(TH_JumpBuffer);
    }

    Super::ExitState();
}

bool UMovementState::OnJumpPressed(const FCommandContext& Ctx)
{
    if (!ownerChar) return false;

    // Record for buffering/coyote (execution happens in containers/modes)
    inputCtx.bWantsJump = true;
    inputCtx.JumpPressedTime = ownerChar->GetWorld()->GetTimeSeconds();

    StartJumpBufferWindow();
    return false; // not consumed; movement containers decide what to do
}

bool UMovementState::OnJumpReleased(const FCommandContext& Ctx)
{
    // IMPORTANT:
    // Do NOT call StopJumping() here.
    // Release behavior is handled by container defaults (Ground/Air) and can be overridden by special substates
    return false;
}

bool UMovementState::OnLookIntent(const FVector2D& Look, const FCommandContext& Ctx)
{
    inputCtx.Look = Look;
    return false;
}

bool UMovementState::OnMoveIntent(const FVector2D& Move, const FCommandContext& Ctx)
{
    inputCtx.Move = Move;
    return false;
}

ILocomotionCmdInterface* UMovementState::GetLocoCmd() const { return ownerStateMachineComp ? ownerStateMachineComp->GetLocomotionCommands() : nullptr; }

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

void UMovementState::ExpireJumpBuffer() { inputCtx.ClearJump(); }

void UMovementState::MarkGroundedNow()
{
    if (ownerChar) lastGroundedTime = ownerChar->GetWorld()->GetTimeSeconds();
}

bool UMovementState::CanUseBufferedJump() const
{
    if (!ownerChar || !moveComp) return false;

    const float Now = ownerChar->GetWorld()->GetTimeSeconds();
    const bool bBuffered = inputCtx.bWantsJump && ((Now - inputCtx.JumpPressedTime) <= jumpBufferSeconds);
    const bool bGroundOrCoyote = moveComp->IsMovingOnGround() || ((Now - lastGroundedTime) <= coyoteSeconds);
    const bool bFirstJumpOnly = (ownerChar->JumpCurrentCount == 0); // prevent consuming into 2nd jump automatically

    return bBuffered && bGroundOrCoyote && bFirstJumpOnly;
}

bool UMovementState::ConsumeBufferedJumpIfValid()
{
    if (!CanUseBufferedJump()) return false;
    inputCtx.ClearJump();
    return true;
}