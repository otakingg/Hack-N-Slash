#pragma once

#include "CoreMinimal.h"
#include "GroundedModeState.h"
#include "GroundLocomotionState.generated.h"

/**
 * Default grounded locomotion behavior (walk/jog/run)
 */
UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UGroundLocomotionState : public UGroundedModeState
{
    GENERATED_BODY()
};