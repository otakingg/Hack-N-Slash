// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/States/Core/CharacterState.h"
#include "CombatState.generated.h"

/**
 * Combat Action base:
 * - Player intent states (attack, block/parry, dodge, etc.)
 * - Defaults to Medium priority (from UActionState)
 * - Generally can be interrupted by Reaction states
 */
UCLASS(Abstract)
class HACK_N_SLASH_API UCombatState : public UActionState
{
    GENERATED_BODY()

public:
    // Combat should usually be interruptible by stronger things (reactions, death, etc.)
    virtual bool CanBeInterruptedBy(const UCharacterState* Other) const override;
};