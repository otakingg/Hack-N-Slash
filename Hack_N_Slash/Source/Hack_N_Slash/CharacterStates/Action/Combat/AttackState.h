// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatState.h"
#include "AttackState.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UAttackState : public UCombatState
{
	GENERATED_BODY()

private:
	bool bSetAirAtkStats = false;

	UPROPERTY(EditDefaultsOnly, Category = "Attack|Air Atk Move Stats")
	float airAtkGravity = 0.0f;

public:
    /* ---------------- Lifecycle ---------------- */
    virtual void EnterState() override;
    virtual void ExitState() override;

    // Animation feedback
    virtual void OnAnimNotify(FGameplayTag NotifyTag) override;
};