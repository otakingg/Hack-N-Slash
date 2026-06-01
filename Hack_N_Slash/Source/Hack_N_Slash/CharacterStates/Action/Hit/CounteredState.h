// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HitState.h"
#include "CounteredState.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UCounteredState : public UHitState
{
	GENERATED_BODY()

public:
    virtual void EnterState() override;
	
};
