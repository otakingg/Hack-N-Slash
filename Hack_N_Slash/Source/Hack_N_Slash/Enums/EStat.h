// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EStat : uint8
{
	AccelerationMax UMETA(DisplayName = "Max Acceleration"),
	SpeedSprint UMETA(DisplayName = "Sprint Speed"),
	SpeedJog UMETA(DisplayName = "Jog Speed"),
	SpeedWalk UMETA(DisplayName = "Walk Speed"),
	SpeedClimb UMETA(DisplayName = "Climb Speed"),
	SpeedGrind UMETA(DisplayName = "Grind Speed"),
	SpeedFly UMETA(DisplayName = "Fly Speed"),
	JumpZVel UMETA(DisplayName = "Jump Z Velocity"),
	HealthMax UMETA(DisplayName = "Max Health"),
	Health UMETA(DisplayName = "Current Health"),
	StaggerMax UMETA(DisplayName = "Max Stagger"),
	Stagger UMETA(DisplayName = "Current Stagger"),
	StaggerRegen UMETA(DisplayName = "Stagger Regen Rate"),
	StaggerRegenDelay UMETA(DisplayName = "Stagger Regen Delay"),
	StaggerStrength UMETA(DisplayName = "Stagger Strength"),
	StaggerResist UMETA(DisplayName = "Stagger Resist"),
	Strength UMETA(DisplayName = "Strength"),
	Defense UMETA(DisplayName = "Defense"),
	Penetration UMETA(DisplayName = "Penetration"),
	CritRate UMETA(DisplayName = "Crit Rate"),
	CritDmg UMETA(DisplayName = "Crit Damage")
};