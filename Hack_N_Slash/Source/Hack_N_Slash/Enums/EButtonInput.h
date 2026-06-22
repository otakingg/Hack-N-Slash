#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EButtonInput : uint8
{
	None                        UMETA(DisplayName = "None"),
	
	FaceEastStarted             UMETA(DisplayName = "East Face Started"),
	FaceEastOngoing             UMETA(DisplayName = "East Face Ongoing"),
	FaceEastCompleted           UMETA(DisplayName = "East Face Completed"),

	FaceWestStarted             UMETA(DisplayName = "West Face Started"),
	FaceWestOngoing             UMETA(DisplayName = "West Face Ongoing"),
	FaceWestCompleted           UMETA(DisplayName = "West Face Completed"),

	FaceNorthStarted            UMETA(DisplayName = "North Face Started"),
	FaceNorthOngoing            UMETA(DisplayName = "North Face Ongoing"),
	FaceNorthCompleted          UMETA(DisplayName = "North Face Completed"),

	FaceSouthStarted            UMETA(DisplayName = "South Face Started"),
	FaceSouthOngoing            UMETA(DisplayName = "South Face Ongoing"),
	FaceSouthCompleted          UMETA(DisplayName = "South Face Completed"),

	MouseTriggered              UMETA(DisplayName = "Mouse is moving"),
	ShoulderLeftStarted         UMETA(DisplayName = "Left Shoulder Started"),
	ShoulderRightStarted        UMETA(DisplayName = "Right Shoulder Started"),
	SpecialLeftStarted          UMETA(DisplayName = "Special Left Started"),
	SpecialRightStarted         UMETA(DisplayName = "Special Right Started"),
	StickButtonLeftStarted      UMETA(DisplayName = "Left Thumbstick Button Started"),
	StickButtonRightStarted     UMETA(DisplayName = "Right Thumbstick Button Started"),
	StickTiltLeftTriggered      UMETA(DisplayName = "Left Thumbstick Tilt Triggered"),
	StickTiltRightTriggered     UMETA(DisplayName = "Right Thumbstick Tilt Triggered"),   
	TriggerLeftStarted          UMETA(DisplayName = "Left Trigger Started"),
	TriggerRightStarted         UMETA(DisplayName = "Right Trigger Started"),
	TriggerRightOngoing         UMETA(DisplayName = "Right Trigger Ongoing")
};