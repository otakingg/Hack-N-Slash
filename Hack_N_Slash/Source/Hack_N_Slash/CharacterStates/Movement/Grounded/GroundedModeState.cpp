// Fill out your copyright notice in the Description page of Project Settings.
#include "GroundedModeState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void UGroundedModeState::EnterState()
{
    Super::EnterState();
}

void UGroundedModeState::ExitState()
{
    Super::ExitState();
}

bool UGroundedModeState::CanEnterState(const UCharacterState* PreviousState) const
{
    if (!ownerChar || !moveComp) return false;

    //Let derived classes decide what “Grounded” means
    return CanEnterGroundedMode(PreviousState);
}


bool UGroundedModeState::CanEnterGroundedMode_Implementation(const UCharacterState* PreviousState) const
{
    //Default is Unreal's defined "grounded"
    //const EMovementMode mode = moveComp ? moveComp->MovementMode.GetValue() : MOVE_None;
    //return (mode == MOVE_Walking) || (mode == MOVE_NavWalking);
    return moveComp ? moveComp->IsMovingOnGround() : false;
}