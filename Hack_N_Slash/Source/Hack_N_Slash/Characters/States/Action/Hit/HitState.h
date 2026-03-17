// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/States/Core/CharacterState.h"
#include "HitState.generated.h"

/**
 * Action-Hit base:
 * - External force states (stagger, knockdown, getup, death,, etc.)
 * - Usually higher priority and more restrictive about being interrupted
 */
UCLASS(Abstract)
class HACK_N_SLASH_API UHitState : public UActionState
{
    GENERATED_BODY()

public:
    // Reactions should not be easily interrupted unless the incoming reaction is stronger
    virtual EStatePriority GetPriority() const override { return EStatePriority::High; }

    // Reactions usually consume player combat intent
    // Movement still can run unless you make a specific reaction consume it
    virtual bool OnAttackIntent(const FVector2D& InputVector) override { return true; }
    virtual bool OnBlockStartIntent() { return true; }
    virtual bool OnBlockStopIntent() { return true; }
    virtual bool OnDodgeIntent(const FVector2D& InputVector) { return true; }
};