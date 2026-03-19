// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../../Core/CharacterState.h"
#include "AirborneModeState.generated.h"

/**
 * Base class for "air modes" hosted by UAirContainerState.
 * Examples: Jump, Falling, Glide, Hover, AirDash, etc.
 *
 * IMPORTANT: This is NOT the air container itself. This is the behavior that runs while airborne.
 */
UCLASS(Abstract)
class HACK_N_SLASH_API UAirborneModeState : public UMovementState
{
    GENERATED_BODY()

protected:
    static constexpr float ZVelEpsilon = 5.0f;
    
    /** Override in derived air modes for special rules (has double jump, stamina, glide unlocked, etc.) */
    UFUNCTION(BlueprintNativeEvent, Category = "Airborne Mode")
    bool CanEnterAirMode(const UCharacterState* PreviousState) const;
    virtual bool CanEnterAirMode_Implementation(const UCharacterState* PreviousState) const;

public:
    virtual void EnterState() override;
    virtual void ExitState() override;

    /** Gatekeeper: only allow these states when actually airborne (unless you relax it in derived classes) */
    virtual bool CanEnterState(const UCharacterState* PreviousState) const override;
};