// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatState.h"
#include "BlockState.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UBlockState : public UCombatState
{
	GENERATED_BODY()

private:
    FTimerHandle TH_PerfectBlock;

    UFUNCTION() void ClearPerfectBlockWindow();

public:
    /* ---------------- Lifecycle ---------------- */
    virtual void EnterState() override;
    virtual void ExitState() override;

	/* ---------------- Intent Hooks (NO TICKING) ---------------- */
	virtual bool OnBlockStopIntent() override;
};
