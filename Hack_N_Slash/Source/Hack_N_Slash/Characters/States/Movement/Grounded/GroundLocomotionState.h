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

    /** Friction */
    UPROPERTY(EditDefaultsOnly, Category="Locomotion|Friction", meta=(ClampMin="0.0"))
    float groundFriction {8.0f};

public:
    virtual void EnterState() override;
    virtual void ExitState() override;

    // Intent hooks
    virtual bool OnLookIntent(const FVector2D& Look) override;
    virtual bool OnMoveIntent(const FVector2D& Move) override;
    virtual bool OnMoveIntent(AActor* Target, const FVector& Loc, const FGameplayTag& MoveProfile, float AcceptanceRadius = 50.0f) override;
};