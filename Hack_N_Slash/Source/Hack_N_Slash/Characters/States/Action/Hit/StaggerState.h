// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/States/Action/Hit/HitState.h"
#include "StaggerState.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UStaggerState : public UHitState
{
	GENERATED_BODY()

protected:
    //UPROPERTY(EditAnywhere, meta=(ClampMin="0.0"))
    //float juggleGravity = 0.2f;
	
public:
    /* ---------------- Lifecycle ---------------- */
    virtual void EnterState() override;
    virtual void ExitState() override;

    /* ---------------- Transition Rules ---------------- */
    //virtual bool CanEnterState(const UCharacterState* PreviousState) const override;
    //virtual bool CanExitState() const override;

    /** Priority-based interruption (still useful for Action layer) */
    //virtual bool CanBeInterruptedBy(const UCharacterState* Other) const override;

	/* ------------------ Combat Feedback ---------------------- */
	virtual void ReceiveHit(const FAtkHitData& HitData) override;
};
