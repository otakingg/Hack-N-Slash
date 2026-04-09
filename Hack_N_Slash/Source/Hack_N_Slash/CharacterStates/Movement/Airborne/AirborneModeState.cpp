// Fill out your copyright notice in the Description page of Project Settings.
#include "AirborneModeState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void UAirborneModeState::EnterState()
{
    Super::EnterState();
}

void UAirborneModeState::ExitState()
{
    Super::ExitState();
}

bool UAirborneModeState::CanEnterState(const UCharacterState* PreviousState) const
{
    if (!ownerChar || !moveComp) return false;
    return CanEnterAirMode(PreviousState); //Let derived classes decide what “Airborne” means
}

bool UAirborneModeState::CanEnterAirMode_Implementation(const UCharacterState* PreviousState) const
{
    //Default is Unreal's defined "falling"
    //const EMovementMode mode = moveComp ? moveComp->MovementMode.GetValue() : MOVE_None;
    //return (mode == MOVE_Falling);
    return moveComp ? moveComp->IsFalling() : false;
}