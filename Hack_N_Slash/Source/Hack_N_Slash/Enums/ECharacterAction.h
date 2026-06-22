#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class ECharacterAction : uint8
{
	None                 UMETA(DisplayName = "None"),
	Attack               UMETA(DisplayName = "Attack"), // Enemy Only
	AttackHeavy          UMETA(DisplayName = "Attack Heavy"), // Player Only
	AttackLight          UMETA(DisplayName = "Attack Light"), // Player Only
	BlockStart           UMETA(DisplayName = "Block Start"),
	BlockStop            UMETA(DisplayName = "Block Stop"),
	ClimbStart           UMETA(DisplayName = "Climb Start"),
	ClimbStop            UMETA(DisplayName = "Climb Stop"),
	Dodge                UMETA(DisplayName = "Dodge"),
	GrindJump            UMETA(DisplayName = "Grind Jump"),
	GrindStart           UMETA(DisplayName = "Grind Start"),
	GrindStop            UMETA(DisplayName = "Grind Stop"),
	JumpStart            UMETA(DisplayName = "Jump Start"),
	JumpStop             UMETA(DisplayName = "Jump Stop"),
	Look                 UMETA(DisplayName = "Look"), // Player Only
	Move                 UMETA(DisplayName = "Move"),
	MoveTo               UMETA(DisplayName = "Move To"),
	Tether               UMETA(DisplayName = "Tether"),
	ToggleLockOnOff      UMETA(DisplayName = "Lock On/Off"), // Player Only
	WallJump             UMETA(DisplayName = "Wall Jump"),
	WallRunStart         UMETA(DisplayName = "Wall Run Start"),
	WallRunStop          UMETA(DisplayName = "Wall Run Start")
};