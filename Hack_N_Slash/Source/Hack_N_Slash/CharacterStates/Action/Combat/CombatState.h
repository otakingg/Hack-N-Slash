// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterStates/Core/CharacterState.h"
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
    // Combat Intents
    virtual bool OnAttackIntent(const FVector2D& InputVector) override;
    //virtual bool OnBlockStartIntent() override { return true; }
    //virtual bool OnBlockStopIntent() override { return true; }
    //virtual bool OnDodgeIntent(const FVector2D& InputVector) override { return true; }

    // Movement Intents
    virtual bool OnJumpStartIntent() override;
    virtual bool OnJumpStopIntent() override;
    virtual bool OnMoveIntent(const FVector2D& InputVector) override { return true; }
    virtual bool OnMoveIntent(AActor* Target, const FVector& Loc = FVector::ZeroVector, float AcceptanceRadius = 50.0f) override { return true; }
};