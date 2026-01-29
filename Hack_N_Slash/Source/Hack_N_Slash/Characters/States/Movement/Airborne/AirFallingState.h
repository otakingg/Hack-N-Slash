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

protected:
    // Optional behavior tuning (air feel)
    UPROPERTY(EditDefaultsOnly, Category="Falling|Rotation")
    bool bUseControllerDesiredRotation {false};

    UPROPERTY(EditDefaultsOnly, Category="Falling|Rotation")
    bool bOrientRotationToMovement {true};

    UPROPERTY(EditDefaultsOnly, Category="Falling|Rotation")
    FRotator rotationRate {FRotator(0.f, 0.f, 360.f)};

public:
    virtual void EnterState() override;

    virtual bool CanEnterAirMode_Implementation(const UCharacterState* PreviousState) const override;
};