#pragma once

#include "CoreMinimal.h"

// Used by the Player Targetting system to determine what logic to use when calculating their soft target
// Align Cam: The player camera is aligned with the target
// Align Move: The move input direction of the player is aligned with the target
// Align Move or Cam: Try Align Move if the move input direction is NON-ZERO, otherwise try Align Cam
// Align Move or Dist: Try Align Move if the move input direction is NON-ZERO, otherwise try Align Dist
// Dist: The target is close enough to the player

UENUM(BlueprintType)
enum class ETargetingStyle : uint8
{
	None UMETA(DisplayName = "None"),
	AlignCam UMETA(DisplayName = "Based on camera direction alignment"),
	AlignMove UMETA(DisplayName = "Based on move direction alignment"),
	AlignMoveOrCam UMETA(DisplayName = "Based on NON-ZERO move direction alignment or camera direction alignment"),
	AlignMoveOrDist UMETA(DisplayName = "Based on NON-ZERO move direction alignment or distance"),
	Dist UMETA(DisplayName = "Based on distance")
};