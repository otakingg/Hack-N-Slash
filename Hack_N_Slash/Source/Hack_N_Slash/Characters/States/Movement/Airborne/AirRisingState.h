// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AirborneModeState.h"
#include "AirRisingState.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UAirRisingState : public UAirborneModeState
{
    GENERATED_BODY()

protected:
    // “Rising” means upward velocity (simple + robust)
    virtual bool CanEnterAirMode_Implementation(const UCharacterState* PreviousState) const override;

public:
    virtual void EnterState() override;
    virtual void OnJumpApexReached() override;
};