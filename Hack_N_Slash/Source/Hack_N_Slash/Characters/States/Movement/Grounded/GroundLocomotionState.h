#pragma once

#include "CoreMinimal.h"
#include "GroundedModeState.h"
#include "GroundLocomotionState.generated.h"

struct FCommandContext;

/**
 * Default grounded locomotion behavior (walk/run).
 * This state configures movement tuning + rotation policy,
 * but delegates actual input application to the locomotion component via interface (Option B).
 */
UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UGroundLocomotionState : public UGroundedModeState
{
    GENERATED_BODY()

protected:
    /** If true, character rotates to movement direction. */
    UPROPERTY(EditDefaultsOnly, Category="Locomotion|Rotation")
    bool bOrientRotationToMovement {true};

    /** If true (and not orient-to-movement), rotation follows controller desired rotation. */
    UPROPERTY(EditDefaultsOnly, Category="Locomotion|Rotation")
    bool bUseControllerDesiredRotation {false};

    UPROPERTY(EditDefaultsOnly, Category="Locomotion|Rotation")
    FRotator rotationRate {FRotator(0.f, 0.0f, 360.f)};

    /** Braking */
    UPROPERTY(EditDefaultsOnly, Category="Locomotion|Braking", meta=(ClampMin="0.0"))
    float brakingDecelerationWalking {2048.f};

public:
    virtual void EnterState() override;
    virtual void ExitState() override;

    // Intent hooks
    virtual bool OnLookIntent(const FVector2D& Look, const FCommandContext& Ctx) override;
    virtual bool OnMoveIntent(const FVector2D& Move, const FCommandContext& Ctx) override;
};