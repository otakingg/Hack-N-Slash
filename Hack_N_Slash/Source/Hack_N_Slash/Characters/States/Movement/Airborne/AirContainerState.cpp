#include "AirContainerState.h"
#include "GameFramework/Character.h"
#include "AirborneModeState.h"
#include "../../../StateMachineComponent.h"

void UAirContainerState::EnterState()
{
    Super::EnterState();

    // On entry, always start in default air mode
    if (defaultAirModeClass) SetSubState(defaultAirModeClass);
}

void UAirContainerState::ExitState()
{
    if (activeSubState)
    {
        activeSubState->ExitState();
        activeSubState = nullptr;
    }

    Super::ExitState();
}

bool UAirContainerState::OnInputJumpPressed()
{
    //Always record input (even if a mode overrides)
    Super::OnInputJumpPressed();

    // 1) Substate can override jump (wallrun/grind/etc)
    if (activeSubState && activeSubState->OnInputJumpPressed()) return true;

    // 2) Default air jump behavior = UE double-jump
    if (ownerChar)
    {
        ownerChar->Jump();
        return true;
    }

    return false;
}

bool UAirContainerState::OnInputJumpReleased()
{
    Super::OnInputJumpReleased();

    // 1) Substate override
    if (activeSubState && activeSubState->OnInputJumpReleased()) return true;

    // 2) Default: preserves variable jump height
    if (ownerChar)
    {
        ownerChar->StopJumping();
        return true;
    }

    return false;
}

bool UAirContainerState::OnInputLook(const FVector2D& Look)
{
    inputCtx.look = Look;
    return activeSubState ? activeSubState->OnInputLook(Look) : false;
}

bool UAirContainerState::OnInputMove(const FVector2D& Move)
{
    inputCtx.move = Move;
    return activeSubState ? activeSubState->OnInputMove(Move) : false;
}

void UAirContainerState::OnLanded(const FHitResult& Hit)
{
    if (activeSubState) activeSubState->OnLanded(Hit);
}

void UAirContainerState::OnMovementModeChanged(ACharacter* InCharacter, EMovementMode PrevMovementMode, uint8 PrevCustomMode)
{
    if (activeSubState) activeSubState->OnMovementModeChanged(InCharacter, PrevMovementMode, PrevCustomMode);
}

void UAirContainerState::RequestAirborneMode(TSubclassOf<UAirborneModeState> ModeClass)
{
    if (!ModeClass) return;
    SetSubState(ModeClass);
}

void UAirContainerState::ClearAirMode()
{
    if (defaultAirModeClass) SetSubState(defaultAirModeClass);
}

void UAirContainerState::SetSubState(TSubclassOf<UAirborneModeState> NewSubStateClass)
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

    UAirborneModeState* NewState = ownerStateMachineComp->GetMovementState<UAirborneModeState>(NewSubStateClass);
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