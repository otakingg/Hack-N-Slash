// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatState.h"
#include "JumpState.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UJumpState : public UCombatState
{
	GENERATED_BODY()

public:
    // Movement Intents
    virtual bool OnMoveIntent(const FVector2D& InputVector) override { return false; }
    virtual bool OnMoveIntent(AActor* Target, const FVector& Loc = FVector::ZeroVector, float AcceptanceRadius = 50.0f) override { return false; }

    // Movement Feedback
    virtual void OnJumpApexReached() override;
};