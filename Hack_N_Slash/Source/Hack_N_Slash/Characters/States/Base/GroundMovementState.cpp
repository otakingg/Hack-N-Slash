// Fill out your copyright notice in the Description page of Project Settings.

#include "GroundMovementState.h"

void UGroundMovementState::EnterState()
{
    Super::EnterState();

    // On entry, always start in default grounded mode
    if (defaultGroundedModeClass) SetSubState(defaultGroundedModeClass);
}

void UGroundMovementState::RequestGroundedMode(TSubclassOf<UGroundMovementState> ModeClass)
{
    if (!ModeClass) return;

    SetSubState(ModeClass);
}

void UGroundMovementState::ClearGroundedMode()
{
    if (defaultGroundedModeClass) SetSubState(defaultGroundedModeClass);
}