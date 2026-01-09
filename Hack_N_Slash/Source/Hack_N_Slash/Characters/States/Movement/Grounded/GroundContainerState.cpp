// Fill out your copyright notice in the Description page of Project Settings.

#include "GroundContainerState.h"
#include "GameFramework/Character.h"
#include "GroundedModeState.h"
#include "../../../StateMachineComponent.h"

void UGroundContainerState::EnterState()
{
    Super::EnterState();

    // If jump was buffered just before landing, execute it now
    if (ownerChar && ConsumeBufferedJumpIfValid())
    {
        ownerChar->Jump();        //UE handles JumpMaxCount + hold time
        //SetSubState(JumpStart); //Optionally set JumpStart mode here instead
        return;
    }

    //Else always start in default grounded mode
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

bool UGroundContainerState::OnInputJumpPressed() { return activeSubState ? activeSubState->OnInputJumpPressed() : false; }

bool UGroundContainerState::OnInputJumpReleased() { return activeSubState ? activeSubState->OnInputJumpReleased() : false; }

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
    //Refresh grounded time (EnterState already does this, but landing is fine too)
    MarkGroundedNow();

    if (ownerChar && ConsumeBufferedJumpIfValid())
    {
        ownerChar->Jump();
        //SetSubState(JumpStart); //Optionally set JumpStart mode here
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

    UGroundedModeState* NewState {ownerStateMachineComp->GetMovementState<UGroundedModeState>(NewSubStateClass)};
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
        UE_LOG(LogTemp, Warning, TEXT("[%s] SetSubState rejected: Can Enter State Failed (%s)."), *GetNameSafe(this), *GetNameSafe(DesiredClass));
        return;
    }

    if (activeSubState) activeSubState->ExitState();
    activeSubState = NewState;
    activeSubState->EnterState();
}