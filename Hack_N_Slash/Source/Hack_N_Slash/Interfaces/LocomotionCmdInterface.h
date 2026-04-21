// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "GameFramework/RootMotionSource.h"
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

    virtual bool CanCoyoteJump() { return false; }
    virtual void MarkGroundedNow() {}

    virtual void AddMoveInput(const FVector2D& Move) {}
    virtual void AddMoveInput(AActor* Target, const FVector& Loc, float AcceptanceRadius = 50.0f) {}

    virtual void JumpStart() {}
    virtual void JumpStop() {}
    virtual void LaunchCharacterHNS(FVector Velocity = FVector::ZeroVector, bool OverrideXY = true, bool OverrideZ = true, float TimeToStop = 0.0f, AActor* Actor = nullptr) {}
    virtual void ApplyRootMotionSource(const FRootMotionSource& RootMotionSrc) {}
    virtual void ClearRootMotionSource() {}
};