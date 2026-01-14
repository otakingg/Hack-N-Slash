#include "CharacterState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "../../StateMachineComponent.h"

/*--------------------------------- UCharacterState ---------------------------------*/
void UCharacterState::Initialize(UStateMachineComponent* InSM, ACharacter* InOwner)
{
    if (bInitialized) return;

    ownerStateMachineComp = InSM;
    ownerChar = InOwner;

    if (ownerStateMachineComp && ownerChar) bInitialized = true;
    else {UE_LOG(LogTemp, Warning, TEXT("[%s] Initialization failed. State Machine Comp and/or Character is null"), *GetNameSafe(this));}
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

    //Initialize grounded time if we enter while grounded
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

bool UMovementState::OnInputJumpPressed()
{
    if (!ownerChar) return false;

    // Record for buffering/coyote (execution happens in containers/modes)
    inputCtx.bWantsJump = true;
    inputCtx.jumpPressedTime = ownerChar->GetWorld()->GetTimeSeconds();

    StartJumpBufferWindow();
    return false;
}

bool UMovementState::OnInputJumpReleased()
{
    // IMPORTANT:
    // Do NOT call StopJumping() here
    // Release behavior is handled by container defaults (Ground/Air) and can be overridden by special substates (wallrun/climb/etc)
    return false;
}

bool UMovementState::OnInputLook(const FVector2D& Look)
{
    inputCtx.look = Look;
    return false;
}

bool UMovementState::OnInputMove(const FVector2D& Move)
{
    inputCtx.move = Move;
    return false;
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

void UMovementState::ExpireJumpBuffer() { inputCtx.ClearJump(); }

void UMovementState::MarkGroundedNow()
{
    if (ownerChar) lastGroundedTime = ownerChar->GetWorld()->GetTimeSeconds();
}

bool UMovementState::CanUseBufferedJump() const
{
    if (!ownerChar || !moveComp) return false;

    const float Now = ownerChar->GetWorld()->GetTimeSeconds();
    const bool bBuffered = inputCtx.bWantsJump && ((Now - inputCtx.jumpPressedTime) <= jumpBufferSeconds);
    const bool bGroundOrCoyote = moveComp->IsMovingOnGround() || ((Now - lastGroundedTime) <= coyoteSeconds);
    const bool bFirstJumpOnly = (ownerChar->JumpCurrentCount == 0); //Prevent buffered/coyote logic from auto-consuming your 2nd jump

    return bBuffered && bGroundOrCoyote && bFirstJumpOnly;
}

bool UMovementState::ConsumeBufferedJumpIfValid()
{
    if (!CanUseBufferedJump()) return false;
    inputCtx.ClearJump();
    return true;
}
/*--------------------------------- UActionState ---------------------------------*/

bool UActionState::OnInputJumpPressed() { return false; }

bool UActionState::OnInputJumpReleased() { return false; }

bool UActionState::OnInputLook(const FVector2D &InputVector) { return false; }

bool UActionState::OnInputMove(const FVector2D &Move) { return false; }