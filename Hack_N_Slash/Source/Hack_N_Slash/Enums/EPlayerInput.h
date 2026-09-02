#pragma once

#include "CoreMinimal.h"

// Represents all the inputs the player can do
// Will be interpreted by the State Machine as an action to try and perform
// EX: JumpStart while climbing could be interpreted as letting go of the surface
UENUM(BlueprintType)
enum class EPlayerInput : uint8
{
	None                      UMETA(DisplayName = "None"),
	AttackHeavyTriggered      UMETA(DisplayName = "Attack Heavy Triggered"),
	AttackHeavyStart          UMETA(DisplayName = "Attack Heavy Start"),
	AttackHeavyOngoing        UMETA(DisplayName = "Attack Heavy Ongoing"),
	AttackHeavyComplete       UMETA(DisplayName = "Attack Heavy Complete"),
	AttackLightTriggered      UMETA(DisplayName = "Attack Light Triggered"),
	AttackLightStart          UMETA(DisplayName = "Attack Light Start"),
	AttackLightOngoing        UMETA(DisplayName = "Attack Light Ongoing"),
	AttackLightComplete       UMETA(DisplayName = "Attack Light Complete"),
	AttackNinjutsuStart       UMETA(DisplayName = "Attack Ninjutsu Start"),
	BlockComplete             UMETA(DisplayName = "Block Complete"),
	BlockStart                UMETA(DisplayName = "Block Start"),
	BlockTrigger              UMETA(DisplayName = "Block Trigger"),
	DodgeStart                UMETA(DisplayName = "Dodge Start"),
	JumpComplete              UMETA(DisplayName = "Jump Complete"),
	JumpStart                 UMETA(DisplayName = "Jump Start"),
	LockOnOffStart            UMETA(DisplayName = "Lock On/Off Start"),
	LookMouseTrigger          UMETA(DisplayName = "Look Mouse Trigger"),
	LookStickTrigger          UMETA(DisplayName = "Look Stick Trigger"),
	MoveTrigger               UMETA(DisplayName = "Move Trigger"),
	UseToolsStart             UMETA(DisplayName = "Use Tools Start")
};