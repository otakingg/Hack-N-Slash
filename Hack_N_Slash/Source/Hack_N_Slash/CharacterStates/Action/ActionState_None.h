// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterStates/Core/CharacterState.h"
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
    
    // Combat Intents
    virtual bool OnAttackIntent(const FVector2D& InputVector) override;
    //virtual bool OnBlockStartIntent() override { return true; }
    //virtual bool OnBlockStopIntent() override { return true; }
    //virtual bool OnDodgeIntent(const FVector2D& InputVector) override { return true; }

    // Movement Intents
    virtual bool OnJumpStartIntent() override;
    virtual bool OnJumpStopIntent() override;
    //virtual bool OnLookIntent(const FVector2D& InputVector) override { return false; }
    //virtual bool OnMoveIntent(const FVector2D& InputVector) override { return true; }
    //virtual bool OnMoveIntent(AActor* Target, const FVector& Loc = FVector::ZeroVector, float AcceptanceRadius = 50.0f) override { return true; }
};