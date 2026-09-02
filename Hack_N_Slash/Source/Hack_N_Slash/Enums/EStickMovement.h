#pragma once

#include "CoreMinimal.h"

// Used by the system to identify the direction and motion of the player's stick input
// Used across the system for things like, attack selection and dodge direction


UENUM(BlueprintType)
enum class EStickDirection : uint8
{
    Any,
    Neutral,

    Forward,
    ForwardRight,
    Right,
    BackRight,
    Back,
    BackLeft,
    Left,
    ForwardLeft,
};

UENUM(BlueprintType)
enum class EStickMotion : uint8
{
    None,
    Circle,

    BackForward,
    ForwardBack,
    LeftRight,
    RightLeft
};

// Defines which stick motions are prioritized if multiple are true
// Performing a circle motion that ends with a forwrad input will mean both are true, but the system will prioritize the circle motion
// Very useful for attack selection
UENUM(BlueprintType)
enum class EStickMovePriority : uint8
{
    Any           = 0,
    OneDirection  = 1,
    TwoDirections = 2,
    Circle        = 3,
};