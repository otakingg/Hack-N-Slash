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
	
public:
	/* ------------------ Combat Feedback ---------------------- */
	virtual void ReceiveHit(const FAtkHitData& HitData) override;
};
