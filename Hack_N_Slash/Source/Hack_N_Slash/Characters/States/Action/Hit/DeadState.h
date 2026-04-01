// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/States/Action/Hit/HitState.h"
#include "DeadState.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UDeadState : public UHitState
{
	GENERATED_BODY()

private:
	int16 animationsPlayed = 0;
	UAnimMontage* montage = nullptr;

public:
    virtual void EnterState() override;
    virtual void ExitState() override;

    // Reactions should not be easily interrupted unless the incoming reaction is stronger
    virtual EStatePriority GetPriority() const override { return EStatePriority::Critical; }

	// Consume look intent
    virtual bool OnLookIntent(const FVector2D& InputVector) override { return true; }

    // Movement feedback
    virtual void OnLanded(const FHitResult& Hit) override;

    // Animation feedback (Action + some Movement like TurnInPlace may care)
    virtual void OnAnimNotify(FName NotifyName) override;
    
	/* ------------------ Combat Feedback ---------------------- */
	virtual void ReceiveHit(const FAtkHitData& HitData) override;
};
