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
    float ClampVelocityOnFinish)
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

UAsyncRootMovement* UAsyncRootMovement::AsyncRootMovement_MoveToDynamic(
    const UObject* WorldContext,
    UCharacterMovementComponent* InCharacterMovement,
    FVector StartLocation,
    AActor* TargetActor,
    float InDuration,
    bool bRestrictSpeedToExpected)
{
    UWorld* ContextWorld = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull);
    if (!ContextWorld || !TargetActor) return nullptr;

    UAsyncRootMovement* Node = NewObject<UAsyncRootMovement>();
    Node->ContextWorld = ContextWorld;
    Node->CharacterMovement = InCharacterMovement;
    Node->Duration = InDuration;
    Node->RegisterWithGameInstance(ContextWorld->GetGameInstance());

    TSharedPtr<FRootMotionSource_MoveToDynamicForce> Source = MakeShared<FRootMotionSource_MoveToDynamicForce>();
    Source->InstanceName = FName("MoveToDynamic");
    Source->AccumulateMode = ERootMotionAccumulateMode::Override;
    Source->Priority = 5;
    Source->StartLocation = StartLocation;
    Source->InitialTargetLocation = TargetActor->GetActorLocation();
    Source->Duration = InDuration;
    Source->bRestrictSpeedToExpected = bRestrictSpeedToExpected;

    Node->ApplyAndTrackRootMotion(Source);

    return Node;
}

UAsyncRootMovement* UAsyncRootMovement::AsyncRootMovement_RadialForce(
    const UObject* WorldContext,
    UCharacterMovementComponent* InCharacterMovement,
    FVector Origin,
    float Radius,
    float Strength,
    float InDuration,
    bool bIsPush,
    UCurveFloat* StrengthOverTime)
{
    UWorld* ContextWorld = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull);
    if (!ContextWorld) return nullptr;

    UAsyncRootMovement* Node = NewObject<UAsyncRootMovement>();
    Node->ContextWorld = ContextWorld;
    Node->CharacterMovement = InCharacterMovement;
    Node->Duration = InDuration;
    Node->RegisterWithGameInstance(ContextWorld->GetGameInstance());

    TSharedPtr<FRootMotionSource_RadialForce> Source = MakeShared<FRootMotionSource_RadialForce>();
    Source->InstanceName = FName("RadialForce");
    Source->AccumulateMode = ERootMotionAccumulateMode::Additive;
    Source->Priority = 5;
    Source->Location = Origin;
    Source->Radius = Radius;
    Source->Strength = Strength;
    Source->Duration = InDuration;
    Source->bIsPush = bIsPush;
    Source->StrengthOverTime = StrengthOverTime;

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