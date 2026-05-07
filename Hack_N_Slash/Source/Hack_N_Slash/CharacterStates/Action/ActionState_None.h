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
    /* ---------------- Lifecycle ---------------- */
    virtual void EnterState();
    //virtual void ExitState();

    /* ---------------- Transition Rules ---------------- */
    virtual EStatePriority GetPriority() const override { return EStatePriority::Low; }
    virtual bool CanBeInterruptedBy(const UCharacterState* Other) const override { return true; }
    
    /* ---------------- Intent Hooks----------------*/
    // Combat Intents
    virtual bool OnAttackIntent(const FVector2D& InputVector, EPlayerAction PlayerAction) override;
    virtual bool OnBlockStartIntent() override;
    virtual bool OnDodgeIntent(const FVector2D& InputVector = FVector2D::ZeroVector) override;

    // Movement Intents
    virtual bool OnJumpStartIntent() override;
    virtual bool OnJumpStopIntent() override;
};