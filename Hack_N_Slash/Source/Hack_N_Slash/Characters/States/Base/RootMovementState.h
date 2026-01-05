// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/States/Base/CharacterState.h"
#include "RootMovementState.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class HACK_N_SLASH_API URootMovementState : public UMovementState
{
    GENERATED_BODY()

protected:
    virtual void ApplyBaselineSubState() override
    {
        EvaluateBaselineSubState();
    }

public:
    virtual void EnterState() override
    {
        Super::EnterState();
        ApplyBaselineSubState(); //Choose grounded vs airborne on entry
    }
};