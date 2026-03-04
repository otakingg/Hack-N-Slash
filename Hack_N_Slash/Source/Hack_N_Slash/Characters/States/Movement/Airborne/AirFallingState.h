// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AirborneModeState.h"
#include "AirFallingState.generated.h"

/**
 * Default falling mode.
 * Container handles movement input (Option A),
 * so this state is mostly for animation/transition rules.
 */
UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UAirFallingState : public UAirborneModeState
{
    GENERATED_BODY()

public:
    virtual bool CanEnterAirMode_Implementation(const UCharacterState* PreviousState) const override;
};