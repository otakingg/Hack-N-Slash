// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../../Core/CharacterState.h"
#include "GroundedModeState.generated.h"

class UCharacterMovementComponent;

/**
 * Base class for "grounded modes" that are hosted by UGroundContainerState.
 * Examples: GroundLocomotion, Climb, Grind, etc.
 *
 * IMPORTANT: This is NOT the grounded container itself. This is the behavior that runs while grounded.
 */
UCLASS(Abstract)
class HACK_N_SLASH_API UGroundedModeState : public UMovementState
{
    GENERATED_BODY()

protected:
    /** Override this in derived grounded modes if you want special rules (rail nearby, ledge detected, etc.) */
    UFUNCTION(BlueprintNativeEvent, Category = "Grounded Mode")
    bool CanEnterGroundedMode(const UCharacterState* PreviousState) const; //Overrideable in Blueprint
    virtual bool CanEnterGroundedMode_Implementation(const UCharacterState* PreviousState) const; //C++ fallback if not overriden

public:
    virtual void EnterState() override;
    virtual void ExitState() override;

    /** Gatekeeper: only allow these states when actually grounded (unless you choose to relax this)*/
	virtual bool CanEnterState(const UCharacterState* PreviousState) const override;
};