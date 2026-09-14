#pragma once

#include "CoreMinimal.h"

// Represents all the chakra natures in the game
UENUM(BlueprintType)
enum class EChakraNature : uint8
{
	None                      UMETA(DisplayName = "None"),
	Air                       UMETA(DisplayName = "Air"),
	Fire                      UMETA(DisplayName = "Fire"),
	Lightning                 UMETA(DisplayName = "Lightning")
};