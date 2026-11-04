#pragma once

#include "CoreMinimal.h"

// Represents all the chakra natures in the game
UENUM(BlueprintType)
enum class EChakraNature : uint8
{
	None                      UMETA(DisplayName = "None"),
	Fire                      UMETA(DisplayName = "Fire"),
	Lightning                 UMETA(DisplayName = "Lightning"),
	Wind                      UMETA(DisplayName = "Wind")
};