#pragma once

#include "CoreMinimal.h"

// Used by the system to identify the direction and motion of the player's stick input
// Used across the system for things like, attack selection and dodge direction
UENUM(BlueprintType)
enum class EMotion : uint8
{
    Any,
    Neutral,

    // 8 cardinal directions
    Forward,
    ForwardRight,
    Right,
    BackRight,
    Back,
    BackLeft,
    Left,
    ForwardLeft,

    // Double direction
    BackForward,
    ForwardBack,
    LeftRight,
    RightLeft,

    // Circle
    Circle
};

// Defines which stick motions are prioritized if multiple are true
// Performing a circle motion that ends with a forwrad input will mean both are true, but the system will prioritize the circle motion
// Very useful for attack selection
UENUM(BlueprintType)
enum class EMotionPriority : uint8
{
    Any           = 0,
    OneDirection  = 1,
    TwoDirections = 2,
    Circle        = 3,
};