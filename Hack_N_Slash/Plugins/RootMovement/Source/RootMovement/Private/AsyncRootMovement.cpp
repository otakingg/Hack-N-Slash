#include "AsyncRootMovement.h"
#include "Engine/Engine.h"

/* ---------------- CONSTANT FORCE ---------------- */

UAsyncRootMovement* UAsyncRootMovement::AsyncRootMovement_ConstantForce(
    const UObject* WorldContext,
    UCharacterMovementComponent* InCharacterMovement,
    FVector Force,
    float Duration,
    bool bAdditive,
    UCurveFloat* StrengthOverTime,
    ERootMotionFinishVelocityMode VelocityOnFinishMode,
    FVector SetVelocityOnFinish,
    float ClampVelocityOnFinish)
{
    UWorld* ContextWorld = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull);
    if (!ContextWorld) return nullptr;

    UAsyncRootMovement* Node = NewObject<UAsyncRootMovement>(); // Creates the actual async action
    Node->ContextWorld = ContextWorld;
    Node->CharacterMovement = InCharacterMovement;
    // This is important for async Blueprint actions
    // It helps Unreal keep the async action alive while it's running
    // Otherwise, you could theoretically create "UAsyncRootMovement", return it to Blueprint, and then have it become eligible for garbage collection before the operation finishes
    // Registering it with the Game Instance gives the async action a lifetime suitable for this pattern
    Node->RegisterWithGameInstance(ContextWorld->GetGameInstance());

    TSharedPtr<FRootMotionSource_ConstantForce> Source = MakeShared<FRootMotionSource_ConstantForce>();
    if (!Source) return nullptr;
    Source->InstanceName = FName("ConstantForce");
    Source->AccumulateMode = bAdditive ? ERootMotionAccumulateMode::Additive : ERootMotionAccumulateMode::Override;
    Source->Priority = 5;
    Source->Force = Force;
    Source->Duration = Duration;
    Source->StrengthOverTime = StrengthOverTime;
    Source->FinishVelocityParams.Mode = VelocityOnFinishMode;
    Source->FinishVelocityParams.SetVelocity = SetVelocityOnFinish;
    Source->FinishVelocityParams.ClampVelocity = ClampVelocityOnFinish;
    
    Node->PendingSource = Source;

    return Node;
}


/* ---------------- JUMP FORCE ---------------- */
UAsyncRootMovement* UAsyncRootMovement::AsyncRootMovement_JumpForce(
    const UObject* WorldContext,
    UCharacterMovementComponent* InCharacterMovement,
    FVector Direction,
    float Distance,
    float Height,
    float Duration,
    ERootMotionFinishVelocityMode VelocityOnFinishMode,
    FVector SetVelocityOnFinish,
    float ClampVelocityOnFinish)
{
    UWorld* ContextWorld = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull);
    if (!ContextWorld) return nullptr;

    UAsyncRootMovement* Node = NewObject<UAsyncRootMovement>();
    Node->ContextWorld = ContextWorld;
    Node->CharacterMovement = InCharacterMovement;
    Node->RegisterWithGameInstance(ContextWorld->GetGameInstance());

    TSharedPtr<FRootMotionSource_JumpForce> Source = MakeShared<FRootMotionSource_JumpForce>();
    if (!Source) return nullptr;
    Source->InstanceName = FName("JumpForce");
    Source->AccumulateMode = ERootMotionAccumulateMode::Override;
    Source->Priority = 5;

    Source->Rotation = Direction.Rotation();
    Source->Distance = Distance;
    Source->Height = Height;
    Source->Duration = Duration;

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
    float Duration,
    bool bRestrictSpeedToExpected)
{
    UWorld* ContextWorld = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull);
    if (!ContextWorld) return nullptr;

    UAsyncRootMovement* Node = NewObject<UAsyncRootMovement>();
    Node->ContextWorld = ContextWorld;
    Node->CharacterMovement = InCharacterMovement;
    Node->RegisterWithGameInstance(ContextWorld->GetGameInstance());

    TSharedPtr<FRootMotionSource_MoveToForce> Source = MakeShared<FRootMotionSource_MoveToForce>();
    if (!Source) return nullptr;
    Source->InstanceName = FName("MoveTo");
    Source->AccumulateMode = ERootMotionAccumulateMode::Override;
    Source->Priority = 5;
    Source->StartLocation = StartLocation;
    Source->TargetLocation = TargetLocation;
    Source->Duration = Duration;
    Source->bRestrictSpeedToExpected = bRestrictSpeedToExpected;

    Node->PendingSource = Source;

    return Node;
}

UAsyncRootMovement* UAsyncRootMovement::AsyncRootMovement_MoveToDynamic(
    const UObject* WorldContext,
    UCharacterMovementComponent* InCharacterMovement,
    FVector StartLocation,
    FVector InitialTargetLocation,
    float Duration,
    bool bRestrictSpeedToExpected)
{
    UWorld* ContextWorld = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull);
    if (!ContextWorld) return nullptr;

    UAsyncRootMovement* Node = NewObject<UAsyncRootMovement>();
    Node->ContextWorld = ContextWorld;
    Node->CharacterMovement = InCharacterMovement;
    Node->RegisterWithGameInstance(ContextWorld->GetGameInstance());

    TSharedPtr<FRootMotionSource_MoveToDynamicForce> Source = MakeShared<FRootMotionSource_MoveToDynamicForce>();
    if (!Source) return nullptr;
    Source->InstanceName = FName("MoveToDynamic");
    Source->AccumulateMode = ERootMotionAccumulateMode::Override;
    Source->Priority = 5;
    Source->StartLocation = StartLocation;
    Source->InitialTargetLocation = InitialTargetLocation;
    Source->Duration = Duration;
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
    float Duration,
    bool bIsPush,
    UCurveFloat* StrengthOverTime)
{
    UWorld* ContextWorld = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull);
    if (!ContextWorld) return nullptr;

    UAsyncRootMovement* Node = NewObject<UAsyncRootMovement>();
    Node->ContextWorld = ContextWorld;
    Node->CharacterMovement = InCharacterMovement;
    Node->RegisterWithGameInstance(ContextWorld->GetGameInstance());

    TSharedPtr<FRootMotionSource_RadialForce> Source = MakeShared<FRootMotionSource_RadialForce>();
    if (!Source) return nullptr;
    Source->InstanceName = FName("RadialForce");
    Source->AccumulateMode = ERootMotionAccumulateMode::Additive;
    Source->Priority = 5;
    Source->Location = Origin;
    Source->Radius = Radius;
    Source->Strength = Strength;
    Source->Duration = Duration;
    Source->bIsPush = bIsPush;
    Source->StrengthOverTime = StrengthOverTime;

    Node->PendingSource = Source;

    return Node;
}

/* ---------------- ACTIVATE ---------------- */

void UAsyncRootMovement::Activate()
{
    ApplyRootMotion();
    PendingSource = nullptr;
}

/* ---------------- CANCEL ---------------- */

void UAsyncRootMovement::Cancel()
{
    if (bWasCancelled) return;
    bWasCancelled = true;

    if (!TH_OnGoing.IsValid()) OnFail.Broadcast(this); // Timer was never set = Failed
    else
    {
        UWorld* World = GetWorld();
        if (World && TH_OnGoing.IsValid()) World->GetTimerManager().ClearTimer(TH_OnGoing);

        if (!CharacterMovement.IsValid()) OnInterrupted.Broadcast(this); // Invalid Character movement = Interrupted
        else
        {
            UCharacterMovementComponent* MoveComp = CharacterMovement.Get();
            if (MoveComp->GetRootMotionSourceByID(RootMotionSourceID).IsValid()) // Valid + Canceling = Interrupted. Root Motion still happening, but we're canceling it
            {
                MoveComp->RemoveRootMotionSourceByID(RootMotionSourceID);
                OnInterrupted.Broadcast(this);
            }
            else OnComplete.Broadcast(this); // Invalid + Canceling = Completed. The Root Motion completed its movement
        }
    }

    Super::Cancel();
}

/* ---------------- HELPERS ---------------- */
void UAsyncRootMovement::ApplyRootMotion()
{
    UWorld* World = GetWorld();
    if (!World || !CharacterMovement.IsValid() || !PendingSource.IsValid()) Cancel();
    else
    {
        UCharacterMovementComponent* MoveComp = CharacterMovement.Get();
        RootMotionSourceID = MoveComp->ApplyRootMotionSource(PendingSource);
        World->GetTimerManager().SetTimer(TH_OnGoing, this, &UAsyncRootMovement::CheckRootMotionStatus, 0.02f, true);
    }
}

void UAsyncRootMovement::CheckRootMotionStatus()
{
    if (bWasCancelled) return;
    else if (!CharacterMovement.IsValid()) Cancel();
    else
    {
        UCharacterMovementComponent* MoveComp = CharacterMovement.Get();
        TSharedPtr<FRootMotionSource> RMS = MoveComp->GetRootMotionSourceByID(RootMotionSourceID);
        if (!RMS.IsValid()) Cancel();
    }
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