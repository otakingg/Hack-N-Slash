// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterStates/Core/CharacterState.h"
#include "LocomotionState.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API ULocomotionState : public UMovementState
{
	GENERATED_BODY()

public:
    /* ---------------- Transition Rules ---------------- */
    virtual bool CanEnterState_Implementation(const UCharacterState* PreviousState) const override;

    /* ---------------- Lifecycle ---------------- */
    virtual void EnterState() override;
    virtual void ExitState() override;
};
