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
    FTimerHandle TH_OnGoing;

    UCharacterMovementComponent* CharacterMovement = nullptr;

    float Duration = 0.f;
    uint16 RootMotionSourceID = 0;

    /** Shared logic */
    void ApplyAndTrackRootMotion(TSharedPtr<FRootMotionSource> Source);

public:
    UPROPERTY(BlueprintAssignable)
    FMovementEvent OnComplete;

    UPROPERTY(BlueprintAssignable)
    FMovementEvent OnFail;

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
        float ClampVelocityOnFinish,
        bool bEnableGravity
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

    // Overrides
    virtual void Activate() override;
    virtual void Cancel() override;

    virtual UWorld* GetWorld() const override
    {
        return ContextWorld.IsValid() ? ContextWorld.Get() : nullptr;
    }
};