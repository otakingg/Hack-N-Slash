#pragma once

#include "CoreMinimal.h"
#include "GroundedModeState.h"
#include "JumpStartState.generated.h"

/**
 * JumpStart = impulse + commitment.
 * - No Tick.
 * - Integrates with UMovementState jump buffering/coyote via ConsumeBufferedJumpIfValid().
 * - Locks movement input by consuming it and/or scaling it.
 * - Allows look input unless disabled.
 * - Exits naturally when movement mode changes to Falling (StateMachine applies baseline Air).
 */
UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UJumpStartState : public UGroundedModeState
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
    FName jumpNotifyName = "Jump";

    /** Movement lock / scaling */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Jump|Control", meta=(Tooltip="commonly true in hack-n-slash games"))
    bool bLockMovementDuringJumpStart = true;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Jump|Control")
    bool bAllowLookDuringJumpStart = true;

public:
    virtual void EnterState() override;
    virtual void ExitState() override;

    // Animation feedback
    virtual void OnAnimNotify(FName NotifyName) override;
};