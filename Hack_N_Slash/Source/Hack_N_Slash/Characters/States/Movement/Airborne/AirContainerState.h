// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../../Core/CharacterState.h"
#include "AirContainerState.generated.h"

class UAirborneModeState;

UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UAirContainerState : public UMovementState
{
    GENERATED_BODY()

protected:
    UPROPERTY()
    TObjectPtr<UAirborneModeState> activeSubState { nullptr };

    /** Default airborne behavior (falling) */
    UPROPERTY(EditDefaultsOnly, Category = "Air", meta = (Tooltip = "Set = Blueprint child of default falling mode"))
    TSubclassOf<UAirborneModeState> defaultAirModeClass; // e.g., Falling state

    void SetSubState(TSubclassOf<UAirborneModeState> NewSubStateClass);

public:
    /** Optional airborne overrides (glide, hover, etc.) */
    void RequestAirborneMode(TSubclassOf<UAirborneModeState> ModeClass);

    /** Return to default air mode */
    void ClearAirMode();

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