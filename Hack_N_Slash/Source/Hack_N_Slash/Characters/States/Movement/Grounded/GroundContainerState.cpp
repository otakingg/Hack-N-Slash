#include "GroundContainerState.h"
#include "GameFramework/Character.h"
#include "GroundedModeState.h"
#include "../../../StateMachineComponent.h"

void UGroundContainerState::EnterState()
{
    Super::EnterState();

    // If jump was buffered just before landing, execute it now
    // (Ground-only auto-consume)
    if (ownerChar && ConsumeBufferedJumpIfValid())
    {
        ownerChar->Jump(); // UE handles JumpMaxCount + hold time
        // SetSubState(JumpStartModeClass); // Optionally switch to JumpStart substate here instead
        return;
    }

    // Else always start in default grounded mode
    if (defaultGroundedModeClass) SetSubState(defaultGroundedModeClass);
}

void UGroundContainerState::ExitState()
{
    if (activeSubState)
    {
        activeSubState->ExitState();
        activeSubState = nullptr;
    }

    Super::ExitState();
}

bool UGroundContainerState::OnInputJumpPressed()
{
    //Always record input (even if a mode overrides)
    Super::OnInputJumpPressed();

    // 1) Give active substate first right of refusal (climb/wallrun/grind/etc later)
    if (activeSubState && activeSubState->OnInputJumpPressed()) return true;

    // 2) Default grounded jump behavior
    if (ownerChar)
    {
        ownerChar->Jump();
        return true;
    }

    return false;
}

bool UGroundContainerState::OnInputJumpReleased()
{
    Super::OnInputJumpReleased();

    // 1) Let substate override release behavior if needed
    if (activeSubState && activeSubState->OnInputJumpReleased()) return true;

    // 2) Default: preserve variable jump height
    if (ownerChar)
    {
        ownerChar->StopJumping();
        return true;
    }

    return false;
}

bool UGroundContainerState::OnInputLook(const FVector2D& Look)
{
    inputCtx.look = Look;
    return activeSubState ? activeSubState->OnInputLook(Look) : false;
}

bool UGroundContainerState::OnInputMove(const FVector2D& Move)
{
    inputCtx.move = Move;
    return activeSubState ? activeSubState->OnInputMove(Move) : false;
}

void UGroundContainerState::OnLanded(const FHitResult& Hit)
{
    // Refresh grounded time for coyote
    MarkGroundedNow();

    // Consume buffered jump on landing (ground-only)
    if (ownerChar && ConsumeBufferedJumpIfValid())
    {
        ownerChar->Jump();
        //SetSubState(JumpStartModeClass);
        return;
    }

    if (activeSubState) activeSubState->OnLanded(Hit);
}

void UGroundContainerState::OnMovementModeChanged(ACharacter* InCharacter, EMovementMode PrevMovementMode, uint8 PrevCustomMode)
{
    if (activeSubState) activeSubState->OnMovementModeChanged(InCharacter, PrevMovementMode, PrevCustomMode);
}

void UGroundContainerState::RequestGroundedMode(TSubclassOf<UGroundedModeState> ModeClass)
{
    if (!ModeClass) return;
    SetSubState(ModeClass);
}

void UGroundContainerState::ClearGroundedMode()
{
    if (defaultGroundedModeClass) SetSubState(defaultGroundedModeClass);
}

void UGroundContainerState::SetSubState(TSubclassOf<UGroundedModeState> NewSubStateClass)
{
    if (!ownerStateMachineComp) return;

    UClass* DesiredClass = NewSubStateClass.Get();
    if (!DesiredClass) return;

    if (DesiredClass->HasAnyClassFlags(CLASS_Abstract))
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] SetSubState rejected: %s is abstract."), *GetNameSafe(this), *GetNameSafe(DesiredClass));
        return;
    }

    if (DesiredClass == GetClass())
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] SetSubState rejected: cannot set substate to self class (%s)."), *GetNameSafe(this), *GetNameSafe(DesiredClass));
        return;
    }

    if (activeSubState && activeSubState->GetClass() == DesiredClass) return;

    UGroundedModeState* NewState = ownerStateMachineComp->GetMovementState<UGroundedModeState>(NewSubStateClass);
    if (!NewState)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] SetSubState failed: no instance found for %s."), *GetNameSafe(this), *GetNameSafe(DesiredClass));
        return;
    }

    if (!NewState->CanEnterState(this))
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] SetSubState rejected: CanEnterState failed (%s)."), *GetNameSafe(this), *GetNameSafe(DesiredClass));
        return;
    }

    if (activeSubState) activeSubState->ExitState();

    activeSubState = NewState;
    activeSubState->EnterState();
}
