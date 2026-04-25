// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatState.h"
#include "DodgeState.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UDodgeState : public UCombatState
{
	GENERATED_BODY()

public:
    /* ---------------- Lifecycle ---------------- */
    //virtual void EnterState() override;
    //virtual void ExitState() override;
};