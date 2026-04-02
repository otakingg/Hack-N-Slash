// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"

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