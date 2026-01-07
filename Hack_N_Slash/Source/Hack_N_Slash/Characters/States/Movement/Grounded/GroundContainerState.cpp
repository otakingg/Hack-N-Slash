// Fill out your copyright notice in the Description page of Project Settings.

#include "GroundContainerState.h"

void UGroundContainerState::EnterState()
{
    Super::EnterState();

    // On entry, always start in default grounded mode
    if (defaultGroundedModeClass) SetSubState(defaultGroundedModeClass);
}

void UGroundContainerState::RequestGroundedMode(TSubclassOf<UMovementState> ModeClass)
{
    if (!ModeClass) return;

    SetSubState(ModeClass);
}

void UGroundContainerState::ClearGroundedMode()
{
    if (defaultGroundedModeClass) SetSubState(defaultGroundedModeClass);
}