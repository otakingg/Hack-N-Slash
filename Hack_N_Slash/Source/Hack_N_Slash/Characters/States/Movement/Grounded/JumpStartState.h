#pragma once

#include "CoreMinimal.h"
#include "GroundedModeState.h"
#include "JumpStartState.generated.h"

/**
 * JumpStart = impulse + commitment.
 * - No Tick.
 * - Integrates with UMovementState jump buffering/coyote via ConsumeBufferedJumpIfValid().
 * - Locks movement input by consuming it (bool return) and/or scaling it.
 * - Allows look input unless disabled.
 * - Exits naturally when movement mode changes to Falling (StateMachine applies baseline Air).
 */
UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UJumpStartState : public UGroundedModeState
{
	GENERATED_BODY()

private:
	bool bImpulseApplied {false};

	void ApplyJumpImpulseOnce();
	void ApplyMoveInputScaled(const FVector2D& Move, float Scale);

protected:
	/** Jump impulse behavior */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Jump|Impulse")
	bool bUseCharacterJumpFunction {true}; // default to true since your system already uses UE Jump() for variable height

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Jump|Impulse")
	float overrideJumpZVelocity {0.0f}; // <=0 uses MoveComp->JumpZVelocity (only used when bUseCharacterJumpFunction = false)

	/** If true, don't apply impulse until takeoffNotifyName is received */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Jump|Anim")
	bool bApplyImpulseOnNotify {false};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Jump|Anim")
	FName takeoffNotifyName {"Jump_Takeoff"};

	/** Movement lock / scaling */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Jump|Control", meta = (Tooltip = "commonly true in hack-n-slash games"))
	bool bLockMovementDuringJumpStart {true};

	// 0.0 = no movement, 0.2 = slight drift, etc.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Jump|Control", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float lockedMoveScale {0.0f};

	// Usually true for hack-n-slash: keep camera responsive during commitment
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Jump|Control")
	bool bAllowLookDuringJumpStart {true};

	virtual bool CanEnterGroundedMode_Implementation(const UCharacterState* PreviousState) const override;

public:
	virtual void EnterState() override;
	virtual void ExitState() override;

	// Input forwarded by UStateMachineComponent (bool = consume)
	virtual bool OnInputLook(const FVector2D& Look) override;
	virtual bool OnInputMove(const FVector2D& Move) override;

	// Event forwarded by UStateMachineComponent
	virtual void OnAnimNotify(FName NotifyName) override;
};