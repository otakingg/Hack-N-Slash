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

    // Hard rule: air modes only while falling / airborne.
    // If you have "glide" implemented as a Custom movement mode,
    // you can expand this check (see note below)
    if (moveComp->IsMovingOnGround()) return false;

    // Optional per-mode rules (double jump available, stamina, ability unlocked, etc.)
    return CanEnterAirMode(PreviousState);
}

bool UAirborneModeState::CanEnterAirMode_Implementation(const UCharacterState* PreviousState) const
{
    return true;
}