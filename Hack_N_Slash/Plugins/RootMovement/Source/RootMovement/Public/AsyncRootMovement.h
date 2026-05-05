#pragma once

#include "CoreMinimal.h"
#include "Engine/CancellableAsyncAction.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/RootMotionSource.h"
#include "AsyncRootMovement.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMovementEvent);

/**
 * Async Root Motion handler with support for multiple source types
 */
UCLASS()
class ROOTMOVEMENT_API UAsyncRootMovement : public UCancellableAsyncAction
{
    GENERATED_BODY()

private:
    TWeakObjectPtr<UWorld> ContextWorld = nullptr;
    TWeakObjectPtr<UCharacterMovementComponent> CharacterMovement = nullptr;
    TSharedPtr<FRootMotionSource> PendingSource = nullptr; // Stored until Activate()

    FTimerHandle TH_OnGoing;

    float Duration = 0.f;
    uint16 RootMotionSourceID = 0;

    bool bWasCancelled = false;

    /** Shared logic */
    void ApplyRootMotion();
    void CheckRootMotionStatus();

public:
    UPROPERTY(BlueprintAssignable)
    FMovementEvent OnComplete;

    UPROPERTY(BlueprintAssignable)
    FMovementEvent OnFail;

    UPROPERTY(BlueprintAssignable)
    FMovementEvent OnInterrupted;

    /** Constant Force */
    UFUNCTION(BlueprintCallable, DisplayName = "Apply Root Motion Constant Force (Async)", meta = (WorldContext = "WorldContext", BlueprintInternalUseOnly = "true"))
    static UAsyncRootMovement* AsyncRootMovement_ConstantForce(
        const UObject* WorldContext,
        UCharacterMovementComponent* CharacterMovement,
        FVector Force,
        float Duration,
        bool bAdditive,
        UCurveFloat* StrengthOverTime,
        ERootMotionFinishVelocityMode VelocityOnFinishMode,
        FVector SetVelocityOnFinish,
        float ClampVelocityOnFinish
    );

    /** Jump Force */
    UFUNCTION(BlueprintCallable, DisplayName = "Apply Root Motion Jump Force (Async)", meta = (WorldContext = "WorldContext", BlueprintInternalUseOnly = "true"))
    static UAsyncRootMovement* AsyncRootMovement_JumpForce(
        const UObject* WorldContext,
        UCharacterMovementComponent* CharacterMovement,
        FVector Direction,
        float Distance = 600.0f,
        float Height = 300.0f,
        float Duration = 0.6f,
        ERootMotionFinishVelocityMode VelocityOnFinishMode = ERootMotionFinishVelocityMode::MaintainLastRootMotionVelocity,
        FVector SetVelocityOnFinish = FVector::ZeroVector,
        float ClampVelocityOnFinish = 0.0f
    );

    /** Move To */
    UFUNCTION(BlueprintCallable, DisplayName = "Apply Root Motion Move To (Async)", meta = (WorldContext = "WorldContext", BlueprintInternalUseOnly = "true"))
    static UAsyncRootMovement* AsyncRootMovement_MoveTo(
        const UObject* WorldContext,
        UCharacterMovementComponent* CharacterMovement,
        FVector StartLocation,
        FVector TargetLocation,
        float Duration,
        bool bRestrictSpeedToExpected = true
    );

    /** Move To Dynamic Target */
    UFUNCTION(BlueprintCallable, DisplayName = "Apply Root Motion Move To Dynamic (Async)", meta = (WorldContext = "WorldContext", BlueprintInternalUseOnly = "true"))
    static UAsyncRootMovement* AsyncRootMovement_MoveToDynamic(
        const UObject* WorldContext,
        UCharacterMovementComponent* CharacterMovement,
        FVector StartLocation,
        FVector InitialTargetLocation,
        float Duration,
        bool bRestrictSpeedToExpected = true
    );

    /** Radial Force */
    UFUNCTION(BlueprintCallable, DisplayName = "Apply Root Motion Radial Force (Async)", meta = (WorldContext = "WorldContext", BlueprintInternalUseOnly = "true"))
    static UAsyncRootMovement* AsyncRootMovement_RadialForce(
        const UObject* WorldContext,
        UCharacterMovementComponent* CharacterMovement,
        FVector Origin,
        float Radius,
        float Strength,
        float Duration,
        bool bIsPush = true,
        UCurveFloat* StrengthOverTime = nullptr
    );

    // Overrides
    virtual void Activate() override;
    virtual void Cancel() override;

    virtual UWorld* GetWorld() const override { return ContextWorld.IsValid() ? ContextWorld.Get() : nullptr;}

    void UpdateMoveToDynamicTargetLocation(FVector NewLoc);
};