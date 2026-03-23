// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/States/Core/CharacterState.h"
#include "ActionState_None.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UActionState_None : public UActionState
{
    GENERATED_BODY()

public:
    virtual EStatePriority GetPriority() const override { return EStatePriority::Low; }

    // This is the key: "None" should be easy to interrupt.
    virtual bool CanBeInterruptedBy(const UCharacterState* Other) const override { return true; }
    
    virtual bool OnAttackIntent(const FVector2D& InputVector) override {return false; }
    virtual bool OnBlockStartIntent() override { return false; }
    virtual bool OnBlockStopIntent() override { return false; }
    virtual bool OnDodgeIntent(const FVector2D& InputVector) override { return false; }
};