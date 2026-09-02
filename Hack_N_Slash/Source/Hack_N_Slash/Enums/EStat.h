#pragma once

#include "CoreMinimal.h"

// Defines all the different stats that an actor can have
// Defense: Reduces the amount of damage taken
// Penetration: Reduces the amount of defense used in damage calculation. Penetration is between 0 and 1, where 0 means no penetration and 1 means full penetration
// Strength: Represents the base damage of the actor

UENUM(BlueprintType)
enum class EStat : uint8
{
	HealthMax UMETA(DisplayName = "Max Health"),
	Health UMETA(DisplayName = "Current Health"),
	Strength UMETA(DisplayName = "Strength"),
	Defense UMETA(DisplayName = "Defense"),
	Penetration UMETA(DisplayName = "Penetration"),
	CritRate UMETA(DisplayName = "Crit Rate"),
	CritDmg UMETA(DisplayName = "Crit Damage")
};