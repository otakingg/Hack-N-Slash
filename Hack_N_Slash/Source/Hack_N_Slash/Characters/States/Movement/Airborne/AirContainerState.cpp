// Fill out your copyright notice in the Description page of Project Settings.
#include "AirContainerState.h"
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

bool UAirContainerState::OnInputJumpPressed() { return activeSubState ? activeSubState->OnInputJumpPressed() : false; }

bool UAirContainerState::OnInputJumpReleased() { return activeSubState ? activeSubState->OnInputJumpReleased() : false; }

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

    UAirborneModeState* NewState { ownerStateMachineComp->GetMovementState<UAirborneModeState>(NewSubStateClass) };
    if (!NewState)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] SetSubState failed: no instance found for %s."), *GetNameSafe(this), *GetNameSafe(DesiredClass));
        return;
    }

    /*if (NewState == this)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] SetSubState rejected: cannot set substate to self instance (%s)."), *GetNameSafe(this), *GetNameSafe(DesiredClass));
        return;
    }*/

    if (!NewState->CanEnterState(this))
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] SetSubState rejected: CanEnterState failed (%s)."), *GetNameSafe(this), *GetNameSafe(DesiredClass));
        return;
    }

    if (activeSubState) activeSubState->ExitState();
    activeSubState = NewState;
    activeSubState->EnterState();
}