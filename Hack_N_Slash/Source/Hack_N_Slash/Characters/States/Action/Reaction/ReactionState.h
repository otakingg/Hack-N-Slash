// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/States/Core/CharacterState.h"
#include "ReactionState.generated.h"

/**
 * Reaction Action base:
 * - External force states (hitreact, stagger, knockdown, getup, death)
 * - Usually higher priority and more restrictive about being interrupted
 */
UCLASS(Abstract)
class HACK_N_SLASH_API UReactionState : public UActionState
{
    GENERATED_BODY()

public:
    // Reactions should not be easily interrupted unless the incoming reaction is stronger.
    virtual EStatePriority GetPriority() const override { return EStatePriority::High; }

    virtual bool CanBeInterruptedBy(const UCharacterState* Other) const override;

    // Reactions usually consume player combat intent.
    // Movement still can run unless you make a specific reaction consume it.
    virtual bool OnAttackIntent(const FCommandContext& Ctx, const FVector2D& InputVector) override { return true; }
    virtual bool OnBlockStartIntent(const FCommandContext& Ctx) { return true; }
    virtual bool OnBlockStopIntent(const FCommandContext& Ctx) { return true; }
    virtual bool OnDodgeIntent(const FCommandContext& Ctx, const FVector2D& InputVector) { return true; }
};