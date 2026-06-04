// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterStates/Core/CharacterState.h"
#include "FallingState.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UFallingState : public UMovementState
{
	GENERATED_BODY()

public:
    /* ---------------- Transition Rules ---------------- */
    virtual bool CanEnterState_Implementation(const UCharacterState* PreviousState) const override;

    /* ---------------- Lifecycle ---------------- */
    virtual void EnterState() override;
};
