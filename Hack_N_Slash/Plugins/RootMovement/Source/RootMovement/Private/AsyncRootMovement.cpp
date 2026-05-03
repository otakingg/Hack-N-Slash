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
    
    Node->PendingSource = Source;

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

    Node->PendingSource = Source;

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

    Node->PendingSource = Source;

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

    Node->PendingSource = Source;

    return Node;
}

/* ---------------- ACTIVATE ---------------- */

void UAsyncRootMovement::Activate()
{
    if (!CharacterMovement.IsValid() || !PendingSource.IsValid())
    {
        OnFail.Broadcast();
        Cancel();
        return;
    }

    ApplyRootMotion();
    PendingSource = nullptr;
}

/* ---------------- Replaces Activate ---------------- */
void UAsyncRootMovement::ApplyRootMotion()
{
    UCharacterMovementComponent* MoveComp = CharacterMovement.Get();
    UWorld* World = GetWorld();

    if (!MoveComp || !World)
    {
        OnFail.Broadcast();
        Cancel();
        return;
    }

    RootMotionSourceID = MoveComp->ApplyRootMotionSource(PendingSource);

    World->GetTimerManager().SetTimer(TH_OnGoing, this, &UAsyncRootMovement::CheckRootMotionStatus, 0.02f, true);
}

void UAsyncRootMovement::CheckRootMotionStatus()
{
    if (!CharacterMovement.IsValid())
    {
        OnInterrupted.Broadcast();
        Cancel();
        return;
    }

    UCharacterMovementComponent* MoveComp = CharacterMovement.Get();
    TSharedPtr<FRootMotionSource> RMS = MoveComp->GetRootMotionSourceByID(RootMotionSourceID);

    if (!RMS.IsValid()) Cancel();
}

/* ---------------- CANCEL ---------------- */

void UAsyncRootMovement::Cancel()
{
    if (bWasCancelled) return;

    bWasCancelled = true;

    UWorld* World = GetWorld();

    if (World && TH_OnGoing.IsValid()) World->GetTimerManager().ClearTimer(TH_OnGoing);

    if (CharacterMovement.IsValid())
    {
        UCharacterMovementComponent* MoveComp = CharacterMovement.Get();

        if (MoveComp->GetRootMotionSourceByID(RootMotionSourceID).IsValid()) // Valid + Canceling = Interrupted
        {
            MoveComp->RemoveRootMotionSourceByID(RootMotionSourceID);
            OnInterrupted.Broadcast();
        }
        else OnComplete.Broadcast(); // Invalid + Canceling = Completed
    }

    Super::Cancel();
}

void UAsyncRootMovement::UpdateMoveToDynamicTargetLocation(FVector NewLoc)
{
    if (!CharacterMovement.IsValid()) return;

    UCharacterMovementComponent* MoveComp = CharacterMovement.Get();
    TSharedPtr<FRootMotionSource> RMS = MoveComp->GetRootMotionSourceByID(RootMotionSourceID);
    if (!RMS.IsValid()) return;

    TSharedPtr<FRootMotionSource_MoveToDynamicForce> MoveToDynamicRMS = StaticCastSharedPtr<FRootMotionSource_MoveToDynamicForce>(RMS);
    if (!MoveToDynamicRMS.IsValid()) return;

    MoveToDynamicRMS->SetTargetLocation(NewLoc);
}