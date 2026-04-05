#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EPlayerAction : uint8
{
	None UMETA(DisplayName = "None"),
	AttackHeavyStart UMETA(DisplayName = "Heavy Attack Start"),
	AttackLightStart UMETA(DisplayName = "Light Attack Start"),
	BlockStart UMETA(DisplayName = "Block Start"),
	BlockStop UMETA(DisplayName = "Block Stop"),
	JumpStart UMETA(DisplayName = "Jump Start"),
	JumpStop UMETA(DisplayName = "Jump Stop")
};