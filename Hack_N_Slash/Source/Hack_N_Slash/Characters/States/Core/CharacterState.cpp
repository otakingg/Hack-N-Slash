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

void UCharacterState::EnterState()
{
    if (bDebug && GEngine)
    {
        const FString ClassName = GetNameSafe(this);
        GEngine->AddOnScreenDebugMessage(
            -1,
            3.f,
            FColor::Blue,
            FString::Printf(TEXT("%s: EnterState"), *ClassName)
        );
    }
}

bool UCharacterState::CanBeInterruptedBy(const UCharacterState* Other) const
{
    if (!Other) return false;
    return Other->GetPriority() >= GetPriority();
}

bool UCharacterState::OnJumpPressed(const FCommandContext& Ctx)
{
    if (bDebug && GEngine)
    {
        const FString ClassName = GetNameSafe(this);
        GEngine->AddOnScreenDebugMessage(
            -1,
            3.f,
            FColor::Blue,
            FString::Printf(TEXT("%s: OnJumpPressed"), *ClassName)
        );
    }
    return false;
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

bool UMovementState::OnJumpPressed(const FCommandContext& Ctx)
{
    Super::OnJumpPressed(Ctx);
    if (!ownerChar) return false;

    if (UWorld* World = ownerChar->GetWorld())
    {
        // Record intent + timestamp (buffer/coyote are checked by consumers later)
        inputCtx.bWantsJump = true;
        inputCtx.JumpPressedTime = World->GetTimeSeconds();
    }

    return false; // not consumed; containers/modes decide what to do
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

ILocomotionCmdInterface* UMovementState::GetLocoCmd() const
{
    return ownerStateMachineComp ? ownerStateMachineComp->GetLocomotionCommands() : nullptr;
}

void UMovementState::MarkGroundedNow()
{
    if (!ownerChar) return;
    if (UWorld* World = ownerChar->GetWorld()) lastGroundedTime = World->GetTimeSeconds();
}

bool UMovementState::CanUseBufferedJump() const
{
    if (!ownerChar || !moveComp) return false;
    UWorld* World = ownerChar->GetWorld();
    if (!World) return false;

    const float Now = World->GetTimeSeconds();

    // Must have a recorded press
    if (!inputCtx.bWantsJump || inputCtx.JumpPressedTime < 0.f) return false;

    // "Buffer" window: how recent the press was
    const bool bBuffered = (Now - inputCtx.JumpPressedTime) <= jumpBufferSeconds;

    // "Coyote" window: how recently we were grounded
    const bool bGroundOrCoyote = moveComp->IsMovingOnGround() || ((Now - lastGroundedTime) <= coyoteSeconds);

    // Prevent consuming into 2nd jump automatically (keeps double jump separate)
    const bool bFirstJumpOnly = (ownerChar->JumpCurrentCount == 0);

    return bBuffered && bGroundOrCoyote && bFirstJumpOnly;
}

bool UMovementState::ConsumeBufferedJumpIfValid()
{
    if (!CanUseBufferedJump()) return false;
    inputCtx.ClearJump();
    return true;
}