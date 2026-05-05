// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "GameFramework/RootMotionSource.h"
#include "AsyncRootMovement.h"
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
    /* ---------------- Tag-driven tuning ---------------- */
    virtual void SetMoveProfileTag(const FGameplayTag& NewProfile) {}
    virtual void AddMoveOverrideTag(const FGameplayTag& OverrideTag) {}
    virtual void RemoveMoveOverrideTag(const FGameplayTag& OverrideTag) {}
    virtual void RefreshMovement() {} // Call when movement-related stats change
    
    /* ---------------- Engine movement mode ---------------- */
    virtual void SetMovementModeCmd(EMovementMode NewMode, uint8 CustomMode = 0) {}

    /* ---------------- Jump buffering / coyote time ----------------*/
    virtual bool CanCoyoteJump() { return false; }
    virtual void MarkGroundedNow() {}

    /* ---------------- Movement Actions ------------------------------*/
    virtual void AddMoveInput(const FVector2D& Move) {}
    virtual void AddMoveInput(AActor* Target, const FVector& Loc, float AcceptanceRadius = 50.0f) {}
    virtual void JumpStart() {}
    virtual void JumpStop() {}
    virtual void LaunchCharacterHNS(FVector Velocity = FVector::ZeroVector, bool OverrideXY = true, bool OverrideZ = true, float TimeToStop = 0.0f, AActor* Actor = nullptr) {}

	/* ---------------- Warping ------------------------------*/
    virtual void GetWarpingLocRot(AActor* Target, FVector& WarpLoc, FRotator& WarpRot, float WarpOffset, const FString& context) {}
	virtual void GetWarpingLocRot(AActor* Target, FVector& WarpLoc, FRotator& WarpRot, float WarpOffset, const FVector2D& InputDir = FVector2D::ZeroVector, bool bLockedOn = false) {}
	virtual void UpdateMotionWarpData(FVector DesiredLoc, FRotator DesiredRot) {}
	virtual void ClearMotionWarpData() {}
    virtual UAsyncRootMovement* ApplyRootMotionSourceConstant(float Duration, FVector Force, FVector VelocityOnFinish = FVector::ZeroVector, float ClampVelocityOnFinish = 0.0f,
        ERootMotionFinishVelocityMode VelocityOnFinishMode = ERootMotionFinishVelocityMode::SetVelocity, UCurveFloat* StrengthOverTime = nullptr, bool bAdditive = false) { return nullptr;}
    virtual UAsyncRootMovement* ApplyRootMotionSourceMoveTo(FVector StartLoc, FVector TargetLoc, float Duration, bool bRestrictSpeedToExpected = true) { return nullptr; }
    virtual UAsyncRootMovement* ApplyRootMotionSourceMoveToDynamic(FVector StartLoc, FVector InitTargetLoc, float Duration, bool bRestrictSpeedToExpected = true) { return nullptr; }
    virtual UAsyncRootMovement* ApplyRootMotionSourceRadial(FVector Origin, float Radius, float Strength, float Duration, bool bIsPush = true, UCurveFloat* StrengthOverTime = nullptr) { return nullptr; }
    virtual void ClearRootMotionSource() {}
    virtual UAsyncRootMovement* GetActiveRootMotionSource() const { return nullptr; }
};