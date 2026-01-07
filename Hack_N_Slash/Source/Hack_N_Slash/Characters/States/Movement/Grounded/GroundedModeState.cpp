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

    // Hard rule: grounded modes only while actually grounded.
    // If you have modes like "ledge hang" that might not count as MovingOnGround,
    // you can loosen/override this in derived classes by overriding CanEnterGroundedMode.
    if (!moveComp->IsMovingOnGround()) return false;

    // Optional per-mode rules (climb needs ledge, grind needs rail, etc.)
    return CanEnterGroundedMode(PreviousState);
}

bool UGroundedModeState::CanEnterGroundedMode_Implementation(const UCharacterState* PreviousState) const
{
    return true;
}