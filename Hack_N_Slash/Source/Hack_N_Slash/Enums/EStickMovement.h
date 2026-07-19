#pragma once

#include "CoreMinimal.h"

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