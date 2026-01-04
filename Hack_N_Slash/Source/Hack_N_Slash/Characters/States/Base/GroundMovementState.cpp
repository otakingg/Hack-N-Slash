// Fill out your copyright notice in the Description page of Project Settings.

#include "GroundMovementState.h"

void UGroundMovementState::EnterState()
{
    Super::EnterState();

    // On entry, always start in default grounded mode
    if (DefaultGroundedModeClass) SetSubState(DefaultGroundedModeClass);
}

void UGroundMovementState::RequestGroundedMode(TSubclassOf<UGroundMovementState> ModeClass)
{
    if (!ModeClass) return;

    SetSubState(ModeClass);
}

void UGroundMovementState::ClearGroundedMode()
{
    if (DefaultGroundedModeClass) SetSubState(DefaultGroundedModeClass);
}