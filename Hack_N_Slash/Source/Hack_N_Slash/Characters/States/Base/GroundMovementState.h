// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/States/Base/CharacterState.h"
#include "GroundMovementState.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UGroundMovementState : public UMovementState
{
    GENERATED_BODY()

protected:
    /** Default grounded behavior (locomotion) */
    UPROPERTY(EditDefaultsOnly, Category="Ground|Defaults", meta = (Tooltip="Set = Locomotion State"))
    TSubclassOf<UGroundMovementState> DefaultGroundedModeClass; //EX: Walk State

public:
    /** Request a grounded override (climb, grind, etc.) */
    void RequestGroundedMode(TSubclassOf<UGroundMovementState> ModeClass);

    /** Return to default grounded mode */
    void ClearGroundedMode();

    virtual void EnterState() override;
};