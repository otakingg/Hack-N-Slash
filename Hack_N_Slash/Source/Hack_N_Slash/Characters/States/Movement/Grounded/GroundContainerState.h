// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../../Core/CharacterState.h"
#include "GroundContainerState.generated.h"

class UGroundedModeState;

UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UGroundContainerState : public UMovementState
{
    GENERATED_BODY()

protected:
    UPROPERTY()
    TObjectPtr<UGroundedModeState> activeSubState {nullptr};

    /** Default grounded behavior (locomotion) */
    UPROPERTY(EditDefaultsOnly, Category = "Ground", meta = (Tooltip = "Set = Blueprint Child of Ground Locomotion State"))
    TSubclassOf<UGroundedModeState> defaultGroundedModeClass; // e.g., Walk/Locomotion state

    UPROPERTY(EditDefaultsOnly, Category = "Ground", meta = (Tooltip = "Set = Blueprint Child of Jump start State"))
    TSubclassOf<UGroundedModeState> jumpStartModeClass;

    void SetSubState(TSubclassOf<UGroundedModeState> NewSubStateClass);

public:
    /** Request a grounded override (climb, grind, etc.) */
    void RequestGroundedMode(TSubclassOf<UGroundedModeState> ModeClass);

    /** Return to default grounded mode */
    void ClearGroundedMode();

    virtual void EnterState() override;
    virtual void ExitState() override;

    // Forward input/events to substate
    virtual bool OnInputJumpPressed() override;
    virtual bool OnInputJumpReleased() override;
    virtual bool OnInputLook(const FVector2D& Look) override;
    virtual bool OnInputMove(const FVector2D& Move) override;

    virtual void OnLanded(const FHitResult& Hit) override;
    virtual void OnMovementModeChanged(ACharacter* InCharacter, EMovementMode PrevMovementMode, uint8 PrevCustomMode) override;
};