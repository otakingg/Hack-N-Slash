// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/States/Action/Combat/CombatState.h"
#include "AttackState.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UAttackState : public UCombatState
{
	GENERATED_BODY()

public:
    //virtual void EnterState() override;
    //virtual void ExitState() override;

    // Action inputs
    virtual bool OnAttackIntent(const FVector2D& InputVector) override;
    virtual bool OnBlockStartIntent() override { return true; }
    virtual bool OnBlockStopIntent() override { return true; }
    virtual bool OnDodgeIntent(const FVector2D& InputVector) override { return true; }

    // Movement Inputs
    virtual bool OnJumpStartIntent() override { return true; }
    virtual bool OnJumpStopIntent() override { return true; }
    virtual bool OnLookIntent(const FVector2D& InputVector) override { return false; }
    virtual bool OnMoveIntent(const FVector2D& InputVector) override { return true; }
    virtual bool OnMoveIntent(AActor* Target, const FVector& Loc = FVector::ZeroVector, float AcceptanceRadius = 50.0f) override { return true; }
};