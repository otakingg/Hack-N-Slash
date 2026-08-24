#pragma once

#include "CoreMinimal.h"
#include "Engine/CancellableAsyncAction.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/RootMotionSource.h"
#include "AsyncRootMovement.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMovementEvent, UAsyncRootMovement*, RootMovement);

/**
 * Async Root Motion handler with support for multiple source types
 */
UCLASS()
class ROOTMOVEMENT_API UAsyncRootMovement : public UCancellableAsyncAction
{
    GENERATED_BODY()

private:
    // If we use Weak Pointers, it's because we don't want this async object to unnecessarily keep the pointer alive
    // That's important because the character could be destroyed while the async operation is running

    TWeakObjectPtr<UWorld> ContextWorld = nullptr; // Needed because the async operation needs access to Timers & the Game Instance
    TWeakObjectPtr<UCharacterMovementComponent> CharacterMovement = nullptr;
    TSharedPtr<FRootMotionSource> PendingSource = nullptr; // Stored until Activate()

    // Used to periodically call "CheckRootMotionStatus()"
    // Eventually the Root Motion Source disappears from the Character Movement Component, wether it finished or was interrupted
    FTimerHandle TH_OnGoing;
    uint16 RootMotionSourceID = 0;

    bool bWasCancelled = false;

    void ApplyRootMotion();
    void CheckRootMotionStatus();

public:
    UPROPERTY(BlueprintAssignable)
    FMovementEvent OnComplete;

    UPROPERTY(BlueprintAssignable)
    FMovementEvent OnFail;

    UPROPERTY(BlueprintAssignable)
    FMovementEvent OnInterrupted;

    // The reason the root motion functions are static is that Blueprint needs a way to create this async action without already having an instance of "UAsyncRootMovement"
    /** Note!!!
     * In Blueprint, "Activate" is called right when the "Apply Root Motion" Node is called
     * In C++, when your create the asyn instance, you have to then call "Activate"
     * EX: UAsyncRootMovement* tempRootMovement = UAsyncRootMovement::AsyncRootMovement_ConstantForce(Parameters); --> tempRootMovement->Activate();
     */

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

    /** Move To Dynamic */
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

    // Helpers
    bool IsActive() const { return !bWasCancelled && CharacterMovement.IsValid() && CharacterMovement->GetRootMotionSourceByID(RootMotionSourceID).IsValid(); }
    void UpdateMoveToDynamicTargetLocation(FVector NewLoc);
};