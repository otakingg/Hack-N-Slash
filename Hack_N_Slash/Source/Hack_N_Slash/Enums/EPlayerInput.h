#pragma once

#include "CoreMinimal.h"

// Represents all the inputs the player can do
// Will be interpreted by the State Machine as an action to try and perform
// EX: JumpStart while climbing could be interpreted as letting go of the surface
UENUM(BlueprintType)
enum class EPlayerInput : uint8
{
	None                      UMETA(DisplayName = "None"),
	AttackHeavyTrigger        UMETA(DisplayName = "Attack Heavy Trigger"),
	AttackHeavyStart          UMETA(DisplayName = "Attack Heavy Start"),
	AttackHeavyOngoing        UMETA(DisplayName = "Attack Heavy Ongoing"),
	AttackHeavyComplete       UMETA(DisplayName = "Attack Heavy Complete"),
	AttackLightTrigger        UMETA(DisplayName = "Attack Light Trigger"),
	AttackLightStart          UMETA(DisplayName = "Attack Light Start"),
	AttackLightOngoing        UMETA(DisplayName = "Attack Light Ongoing"),
	AttackLightComplete       UMETA(DisplayName = "Attack Light Complete"),
	AttackNinjutsuTrigger     UMETA(DisplayName = "Attack Ninjutsu Trigger"),
	AttackNinjutsuStart       UMETA(DisplayName = "Attack Ninjutsu Start"),
	AttackNinjutsuOngoing     UMETA(DisplayName = "Attack Ninjutsu Ongoing"),
	AttackNinjutsuComplete    UMETA(DisplayName = "Attack Ninjutsu Complete"),
	DefendTrigger			  UMETA(DisplayName = "Defend Trigger"),
	DefendStart				  UMETA(DisplayName = "Defend Start"),
	DefendComplete			  UMETA(DisplayName = "Defend Complete"),
	JumpStart                 UMETA(DisplayName = "Jump Start"),
	JumpComplete              UMETA(DisplayName = "Jump Complete"),
	LockOnOffStart            UMETA(DisplayName = "Lock On/Off Start"),
	LookMouseTrigger          UMETA(DisplayName = "Look Mouse Trigger"),
	LookStickTrigger          UMETA(DisplayName = "Look Stick Trigger"),
	MoveTrigger               UMETA(DisplayName = "Move Trigger"),
	RegenChakraTrigger        UMETA(DisplayName = "Regen Chakra Trigger"),
	RegenChakraComplete       UMETA(DisplayName = "Regen Chakra Complete"),
	SwitchTargetLeftStart     UMETA(DisplayName = "Switch Target Left Start"),
	SwitchTargetRightStart    UMETA(DisplayName = "Switch Target Right Start")
};

/*UENUM(BlueprintType)
enum class EPlayerInput : uint8
{
	None                           UMETA(DisplayName = "None"),
	AttackHeavyTrigger             UMETA(DisplayName = "Attack Heavy Trigger"),
	AttackHeavyStart               UMETA(DisplayName = "Attack Heavy Start"),
	AttackHeavyOngoing             UMETA(DisplayName = "Attack Heavy Ongoing"),
	AttackHeavyComplete            UMETA(DisplayName = "Attack Heavy Complete"),
	AttackLightTrigger             UMETA(DisplayName = "Attack Light Trigger"),
	AttackLightStart               UMETA(DisplayName = "Attack Light Start"),
	AttackLightOngoing             UMETA(DisplayName = "Attack Light Ongoing"),
	AttackLightComplete            UMETA(DisplayName = "Attack Light Complete"),
	AttackNinjutsuTrigger          UMETA(DisplayName = "Attack Ninjutsu Trigger"),
	AttackNinjutsuStart            UMETA(DisplayName = "Attack Ninjutsu Start"),
	AttackNinjutsuOngoing          UMETA(DisplayName = "Attack Ninjutsu Ongoing"),
	AttackNinjutsuComplete         UMETA(DisplayName = "Attack Ninjutsu Complete"),
	AttackNinjutsuKBCircleStart    UMETA(DisplayName = "Attack Ninjutsu Start + Circle"), // Keyboard can't do circle motion, so this represents that action
	AttackNinjutsuKBCircleComplete UMETA(DisplayName = "Attack Ninjutsu Complete + Circle"), // Keyboard can't do circle motion, so this represents that action
	BlockTrigger                   UMETA(DisplayName = "Block Trigger"),
	BlockStart                     UMETA(DisplayName = "Block Start"),
	BlockComplete                  UMETA(DisplayName = "Block Complete"),
	DodgeStart                     UMETA(DisplayName = "Dodge Start"),
	JumpStart                      UMETA(DisplayName = "Jump Start"),
	JumpComplete                   UMETA(DisplayName = "Jump Complete"),
	LockOnOffStart                 UMETA(DisplayName = "Lock On/Off Start"),
	LookMouseTrigger               UMETA(DisplayName = "Look Mouse Trigger"),
	LookStickTrigger               UMETA(DisplayName = "Look Stick Trigger"),
	MoveTrigger                    UMETA(DisplayName = "Move Trigger"),
	RegenChakraTrigger             UMETA(DisplayName = "Regen Chakra Trigger"),
	RegenChakraComplete            UMETA(DisplayName = "Regen Chakra Complete"),
	SwitchTargetLeftStart          UMETA(DisplayName = "Switch Target Left Start"),
	SwitchTargetRightStart         UMETA(DisplayName = "Switch Target Right Start")
};*/