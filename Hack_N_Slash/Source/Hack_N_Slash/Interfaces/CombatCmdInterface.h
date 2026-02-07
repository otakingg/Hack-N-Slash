// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CombatCmdInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UCombatCmdInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class HACK_N_SLASH_API ICombatCmdInterface
{
    GENERATED_BODY()

public:
    virtual void AttackIntent(const FVector2D& Dir) {}
    virtual void BlockDodgeIntent(const FVector2D& Dir) {}
};