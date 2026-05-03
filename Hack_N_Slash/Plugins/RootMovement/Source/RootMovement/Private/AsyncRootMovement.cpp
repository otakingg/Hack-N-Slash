#include "AsyncRootMovement.h"
#include "Engine/Engine.h"

/* ---------------- CONSTANT FORCE ---------------- */

UAsyncRootMovement* UAsyncRootMovement::AsyncRootMovement_ConstantForce(
    const UObject* WorldContext,
    UCharacterMovementComponent* InCharacterMovement,
    FVector Force,
    float InDuration,
    bool bAdditive,
    UCurveFloat* StrengthOverTime,
    ERootMotionFinishVelocityMode VelocityOnFinishMode,
    FVector SetVelocityOnFinish,
    float ClampVelocityOnFinish,
    bool bEnableGravity)
{
    UWorld* ContextWorld = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull);
    if (!ContextWorld) return nullptr;

    UAsyncRootMovement* Node = NewObject<UAsyncRootMovement>();
    Node->ContextWorld = ContextWorld;
    Node->CharacterMovement = InCharacterMovement;
    Node->Duration = InDuration;
    Node->RegisterWithGameInstance(ContextWorld->GetGameInstance());

    TSharedPtr<FRootMotionSource_ConstantForce> Source = MakeShared<FRootMotionSource_ConstantForce>();
    Source->InstanceName = FName("ConstantForce");
    Source->AccumulateMode = bAdditive ? ERootMotionAccumulateMode::Additive : ERootMotionAccumulateMode::Override;
    Source->Priority = 5;
    Source->Force = Force;
    Source->Duration = InDuration;
    Source->StrengthOverTime = StrengthOverTime;
    Source->FinishVelocityParams.Mode = VelocityOnFinishMode;
    Source->FinishVelocityParams.SetVelocity = SetVelocityOnFinish;
    Source->FinishVelocityParams.ClampVelocity = ClampVelocityOnFinish;

    if (!bEnableGravity) Source->Settings.SetFlag(ERootMotionSourceSettingsFlags::IgnoreZAccumulate);
    
    Node->ApplyAndTrackRootMotion(Source);

    return Node;
}

/* ---------------- MOVE TO ---------------- */

UAsyncRootMovement* UAsyncRootMovement::AsyncRootMovement_MoveTo(
    const UObject* WorldContext,
    UCharacterMovementComponent* InCharacterMovement,
    FVector StartLocation,
    FVector TargetLocation,
    float InDuration,
    bool bRestrictSpeedToExpected)
{
    UWorld* ContextWorld = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull);
    if (!ContextWorld) return nullptr;

    UAsyncRootMovement* Node = NewObject<UAsyncRootMovement>();
    Node->ContextWorld = ContextWorld;
    Node->CharacterMovement = InCharacterMovement;
    Node->Duration = InDuration;
    Node->RegisterWithGameInstance(ContextWorld->GetGameInstance());

    TSharedPtr<FRootMotionSource_MoveToForce> Source = MakeShared<FRootMotionSource_MoveToForce>();
    Source->InstanceName = FName("MoveTo");
    Source->AccumulateMode = ERootMotionAccumulateMode::Override;
    Source->Priority = 5;
    Source->StartLocation = StartLocation;
    Source->TargetLocation = TargetLocation;
    Source->Duration = InDuration;
    Source->bRestrictSpeedToExpected = bRestrictSpeedToExpected;

    Node->ApplyAndTrackRootMotion(Source);

    return Node;
}

/* ---------------- Replaces Activate ---------------- */
void UAsyncRootMovement::ApplyAndTrackRootMotion(TSharedPtr<FRootMotionSource> Source)
{
    const UWorld* world = GetWorld();
    if (!world || !CharacterMovement || !Source.IsValid())
    {
        OnFail.Broadcast();
        Cancel();
        return;
    }

    RootMotionSourceID = CharacterMovement->ApplyRootMotionSource(Source);

    FTimerManager& TimerManager = world->GetTimerManager();
    TimerManager.SetTimer(
        TH_OnGoing,
        FTimerDelegate::CreateLambda([WeakThis = TWeakObjectPtr<UAsyncRootMovement>(this)]()
        {
            if (WeakThis.IsValid() && WeakThis->IsActive())
            {
                WeakThis->OnComplete.Broadcast();
                WeakThis->Cancel();
            }
        }),
        Duration,
        false
    );
}

/* ---------------- ACTIVATE ---------------- */

void UAsyncRootMovement::Activate()
{
    // We don't use Activate anymore since everything runs in factory functions
    if (!CharacterMovement)
    {
        OnFail.Broadcast();
        Cancel();
    }
}

/* ---------------- CANCEL ---------------- */

void UAsyncRootMovement::Cancel()
{
    Super::Cancel();

    const UWorld* world = GetWorld();
    if (!world) return;

    if (TH_OnGoing.IsValid())
    {
        if (CharacterMovement) CharacterMovement->RemoveRootMotionSourceByID(RootMotionSourceID);
        world->GetTimerManager().ClearTimer(TH_OnGoing);
    }
}