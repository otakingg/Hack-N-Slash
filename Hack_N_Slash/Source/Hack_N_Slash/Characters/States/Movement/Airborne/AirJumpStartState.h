// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AirborneModeState.h"
#include "AirJumpStartState.generated.h"

struct FCommandContext;

/**
 * Used for coyte jump, because technically you're airborne the moment you leave the ground so this was needed as a parallel to the grounded jump start state
 * JumpStart = impulse + commitment.
 * - No Tick.
 * - Integrates with UMovementState jump buffering/coyote via ConsumeBufferedJumpIfValid().
 * - Locks movement input by consuming it and/or scaling it.
 * - Allows look input unless disabled.
 * - Exits naturally when movement mode changes to Falling (StateMachine applies baseline Air).
 */
UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UAirJumpStartState : public UAirborneModeState
{
    GENERATED_BODY()

private:
    bool bImpulseApplied = false;

    void ApplyJumpImpulseOnce();

protected:
    /** Jump impulse behavior */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Jump|Impulse")
    bool bUseCharacterJumpFunction = true;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Jump|Impulse")
    float overrideJumpZVelocity = 0.0f; // <=0 uses MoveComp->JumpZVelocity (only used when bUseCharacterJumpFunction = false)

    /** If true, don't apply impulse until takeoffNotifyName is received */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Jump|Anim")
    bool bApplyImpulseOnNotify = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Jump|Anim")
    FName takeoffNotifyName = "Jump";

    /** Movement lock / scaling */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Jump|Control", meta=(Tooltip="commonly true in hack-n-slash games"))
    bool bLockMovementDuringJumpStart = true;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Jump|Control", meta=(ClampMin="0.0", ClampMax="1.0"))
    float lockedMoveScale = 0.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Jump|Control")
    bool bAllowLookDuringJumpStart = true;

public:
    virtual void EnterState() override;
    virtual void ExitState() override;

    // Intent hooks (bool = consume)
    virtual bool OnLookIntent(const FVector2D& Look) override;
    virtual bool OnMoveIntent(const FVector2D& Move) override;
    virtual bool OnMoveIntent(const FGameplayTag& MoveProfile, AActor* Target, const FVector& Loc = FVector::ZeroVector, float AcceptanceRadius = 50.0f) override;

    // Animation feedback
    virtual void OnAnimNotify(FName NotifyName) override;
};
