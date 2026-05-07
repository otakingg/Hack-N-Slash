// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HitState.h"
#include "BlockBreakState.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UBlockBreakState : public UHitState
{
	GENERATED_BODY()

public:
    /* ---------------- Lifecycle ---------------- */
    //virtual void EnterState() override;
    //virtual void ExitState() override;

    /* ---------------- Transition Rules ---------------- */
    //virtual bool CanEnterState(const UCharacterState* PreviousState) const override;
    //virtual bool CanExitState() const override;

    /** Priority-based interruption (still useful for Action layer) */
    //virtual bool CanBeInterruptedBy(const UCharacterState* Other) const override;

	/* ------------------ Movement Feedback ---------------------- */
    //virtual void OnLanded(const FHitResult& Hit) override;
    //virtual void OnMovementModeChanged(ACharacter* InCharacter, EMovementMode PrevMovementMode, uint8 PrevCustomMode) {}

    /* ------------------ Animation Feedback ---------------------- */
    //virtual void OnAnimNotify(FName NotifyName) override;

	/* ------------------ Combat Feedback ---------------------- */
	virtual void ReceiveHit(const FAtkHitData& HitData) override;
};
