#include "CharacterState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../../StateMachineComponent.h" // For FCommandContext + interface access if needed
#include "../../Interfaces/LocomotionCmdInterface.h" // For ILocomotionCmdInterface

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
        GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, FString::Printf(TEXT("%s: EnterState"), *ClassName));
    }
}

bool UCharacterState::CanBeInterruptedBy(const UCharacterState* Other) const
{
    if (!Other) return false;
    return Other->GetPriority() >= GetPriority();
}

bool UCharacterState::OnJumpPressed()
{
    if (bDebug && GEngine)
    {
        const FString ClassName = GetNameSafe(this);
        GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, FString::Printf(TEXT("%s: OnJumpPressed"), *ClassName));
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
    if (ownerChar && moveComp && moveComp->IsMovingOnGround())
    {
        if (ILocomotionCmdInterface* locoCmd = GetLocoCmd()) locoCmd->MarkGroundedNow();
    }
}

void UMovementState::ClearAirborneModeDelayed()
{
    FTimerManager& TimerManager = ownerChar->GetWorld()->GetTimerManager();
    if (TimerManager.IsTimerActive(TH_ClearAirborne)) TimerManager.ClearTimer(TH_ClearAirborne);
    TimerManager.SetTimer(TH_ClearAirborne, ownerStateMachineComp, &UStateMachineComponent::ClearAirborneMode, 0.1f,false);
}

bool UMovementState::OnJumpPressed()
{
    Super::OnJumpPressed();
    if (!ownerChar) return false;

    if (UWorld* World = ownerChar->GetWorld())
    {
        // Record intent + timestamp (buffer/coyote are checked by consumers later)
        inputCtx.bWantsJump = true;
        inputCtx.JumpPressedTime = World->GetTimeSeconds();
    }

    return false; // not consumed; containers/modes decide what to do
}

bool UMovementState::OnJumpReleased()
{
    // IMPORTANT:
    // Do NOT call StopJumping() here.
    // Release behavior is handled by container defaults (Ground/Air) and can be overridden by special substates
    return false;
}

bool UMovementState::OnLookIntent(const FVector2D& Look)
{
    inputCtx.Look = Look;
    return false;
}

bool UMovementState::OnMoveIntent(const FVector2D& Move)
{
    inputCtx.Move = Move;
    return false;
}

bool UMovementState::OnMoveIntent(AActor *Target, const FVector &Loc, float AcceptanceRadius) { return false; }

ILocomotionCmdInterface* UMovementState::GetLocoCmd() const
{
    return ownerStateMachineComp ? ownerStateMachineComp->GetLocomotionCommands() : nullptr;
}

bool UMovementState::ConsumeBufferedJumpIfValid()
{
    ILocomotionCmdInterface* locoCmd = GetLocoCmd();
    if (!locoCmd) return false;

    if (!locoCmd->CanUseBufferedJump(inputCtx.bWantsJump, inputCtx.JumpPressedTime)) return false;
    inputCtx.ClearJump();
    return true;
}