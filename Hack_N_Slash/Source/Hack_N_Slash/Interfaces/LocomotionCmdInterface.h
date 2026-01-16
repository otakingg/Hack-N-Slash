// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "LocomotionCmdInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class ULocomotionCmdInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class HACK_N_SLASH_API ILocomotionCmdInterface
{
    GENERATED_BODY()
public:
    virtual void AddMoveInputScaled(const FVector2D& Move, float Scale) {}
    virtual void AddLookInputScaled(const FVector2D& Look, float YawRate, float PitchRate) {}

    virtual void JumpPressed() {}
    virtual void JumpReleased() {}
    virtual void LaunchUp(float JumpZ) {}
};
