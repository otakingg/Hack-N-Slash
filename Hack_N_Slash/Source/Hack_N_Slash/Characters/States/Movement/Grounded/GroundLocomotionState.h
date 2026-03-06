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

public:
    // Intent hooks
    virtual bool OnLookIntent(const FVector2D& Look) override;
    virtual bool OnMoveIntent(const FVector2D& Move) override;
    virtual bool OnMoveIntent(const FGameplayTag& MoveProfile, AActor* Target, const FVector& Loc = FVector::ZeroVector, float AcceptanceRadius = 50.0f) override;
};