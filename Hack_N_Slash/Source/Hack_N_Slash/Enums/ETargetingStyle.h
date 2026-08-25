#pragma once

#include "CoreMinimal.h"


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