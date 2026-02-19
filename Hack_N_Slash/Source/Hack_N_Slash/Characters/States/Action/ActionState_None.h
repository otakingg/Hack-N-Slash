// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/States/Core/CharacterState.h"
#include "ActionState_None.generated.h"

/**
 * 
 */
UCLASS()
class HACK_N_SLASH_API UActionState_None : public UActionState
{
    GENERATED_BODY()

public:
    virtual EStatePriority GetPriority() const override { return EStatePriority::Low; }

    // Default should basically never block transitions out.
    virtual bool CanExitState() const override { return true; }

    // This is the key: "None" should be easy to interrupt.
    virtual bool CanBeInterruptedBy(const UCharacterState* Other) const override { return true; }
    
    virtual bool OnAttackIntent(const FCommandContext& Ctx, const FVector2D& InputVector) override {return false; }
    virtual bool OnBlockStartIntent(const FCommandContext& Ctx) { return false; }
    virtual bool OnBlockStopIntent(const FCommandContext& Ctx) { return false; }
    virtual bool OnDodgeIntent(const FCommandContext& Ctx, const FVector2D& InputVector) { return false; }
};