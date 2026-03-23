// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
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
    virtual void SetMoveProfileTag(const FGameplayTag& NewProfile) {}
    virtual void AddMoveOverrideTag(const FGameplayTag& OverrideTag) {}
    virtual void RemoveMoveOverrideTag(const FGameplayTag& OverrideTag) {}
    virtual void RefreshMovement() {} // Call when movement-related stats change

    virtual void SetMovementModeCmd(EMovementMode NewMode, uint8 CustomMode = 0) {}
    
    virtual bool CanMultiJump() const { return false; }
    virtual bool CanUseBufferedJump(bool& bWantsJump, float& JumpPressedTime) const { return false; }
    virtual void MarkGroundedNow() {}

    virtual void AddMoveInputScaled(const FVector2D& Move, float Scale = 1.f) {}
    virtual void AddMoveInputScaled(AActor* Target, const FVector& Loc, float AcceptanceRadius = 50.0f, float Scale = 1.0f) {}

    virtual void JumpPressed() {}
    virtual void JumpReleased() {}
    virtual void LaunchCharacterHNS(FVector Velocity = FVector::ZeroVector, bool OverrideXY = true, bool OverrideZ = true, float TimeToStop = 0.0f, AActor* Actor = nullptr) {}
};