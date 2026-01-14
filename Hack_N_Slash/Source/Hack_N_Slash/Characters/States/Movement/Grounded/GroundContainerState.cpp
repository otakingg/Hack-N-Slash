#include "GroundContainerState.h"
#include "GameFramework/Character.h"
#include "GroundedModeState.h"
#include "../../../StateMachineComponent.h"

void UGroundContainerState::EnterState()
{
    Super::EnterState();

    // If jump was buffered just before landing, execute it now (ground-only)
    if (ownerChar && ConsumeBufferedJumpIfValid())
    {
        if (jumpStartModeClass) SetSubState(jumpStartModeClass);
        else ownerChar->Jump(); // Fallback
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
    Super::OnInputJumpPressed();

    if (!ownerChar) return false;

    // 1) Give active substate first right of refusal (climb/wallrun/grind/etc)
    if (activeSubState && activeSubState->OnInputJumpPressed()) return true;

    // 2) Default grounded jump behavior: prefer JumpStart, fallback to Jump()
    if (jumpStartModeClass)
    {
        SetSubState(jumpStartModeClass);
        return true;
    }

    ownerChar->Jump(); // Fallback
    return true;
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
    MarkGroundedNow();

    if (ownerChar && ConsumeBufferedJumpIfValid())
    {
        if (jumpStartModeClass) SetSubState(jumpStartModeClass);
        else ownerChar->Jump(); // Fallback
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

    if (activeSubState && activeSubState->GetClass() == DesiredClass) return;

    UGroundedModeState* NewState = ownerStateMachineComp->GetMovementState<UGroundedModeState>(NewSubStateClass);
    if (!NewState)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] SetSubState failed: no instance found for %s."), *GetNameSafe(this), *GetNameSafe(DesiredClass));
        return;
    }

    const UCharacterState* prev = activeSubState ? Cast<UCharacterState>(activeSubState) : Cast<UCharacterState>(this);
    if (!NewState->CanEnterState(prev))
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] SetSubState rejected: CanEnterState failed (%s)."), *GetNameSafe(this), *GetNameSafe(DesiredClass));
        return;
    }

    if (activeSubState) activeSubState->ExitState();

    activeSubState = NewState;
    activeSubState->EnterState();
}
