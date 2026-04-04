#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EButtonInput : uint8
{
	None UMETA(DisplayName = "None"),
	FaceEast UMETA(DisplayName = "East Face"),
	FaceWest UMETA(DisplayName = "West Face"),
	FaceNorth UMETA(DisplayName = "North Face"),
	FaceSouth UMETA(DisplayName = "South Face"),
	ShoulderLeft UMETA(DisplayName = "Left Shoulder"),
	ShoulderRight UMETA(DisplayName = "Right Shoulder"),
	SpecialLeft UMETA(DisplayName = "Special Left"),
	SpecialRight UMETA(DisplayName = "Special Right"),
	ThumbStickLeft UMETA(DisplayName = "Left Thumbstick"),
	ThumbStickRight UMETA(DisplayName = "Right Thumbstick"),
	TriggerLeft UMETA(DisplayName = "Left Trigger"),
	TriggerRight UMETA(DisplayName = "Right Trigger"),
};