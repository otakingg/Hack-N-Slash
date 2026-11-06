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
    FVector VelocityOnFinish,
    float ClampVelocityOnFinish)
{
    UWorld* world = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull);
    if (!world) return nullptr;

    UAsyncRootMovement* Node = NewObject<UAsyncRootMovement>(); // Creates the actual async action
    Node->world = world;
    Node->charMoveComp = InCharacterMovement;
    // This is important for async Blueprint actions
    // It helps Unreal keep the async action alive while it's running
    // Otherwise, you could theoretically create "UAsyncRootMovement", return it to Blueprint, and then have it become eligible for garbage collection before the operation finishes
    // Registering it with the Game Instance gives the async action a lifetime suitable for this pattern
    Node->RegisterWithGameInstance(world->GetGameInstance());

    TSharedPtr<FRootMotionSource_ConstantForce> source = MakeShared<FRootMotionSource_ConstantForce>();
    if (!source.IsValid()) return nullptr;

    source->InstanceName = FName("ConstantForce");
    source->AccumulateMode = bAdditive ? ERootMotionAccumulateMode::Additive : ERootMotionAccumulateMode::Override;
    source->Priority = 5;
    source->Force = Force;
    source->Duration = Duration;
    source->StrengthOverTime = StrengthOverTime;
    source->FinishVelocityParams.Mode = VelocityOnFinishMode;
    source->FinishVelocityParams.SetVelocity = VelocityOnFinish;
    source->FinishVelocityParams.ClampVelocity = ClampVelocityOnFinish;
    
    Node->pendingSource = source;

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
    FVector VelocityOnFinish,
    float ClampVelocityOnFinish)
{
    UWorld* world = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull);
    if (!world) return nullptr;

    UAsyncRootMovement* Node = NewObject<UAsyncRootMovement>();
    Node->world = world;
    Node->charMoveComp = InCharacterMovement;
    Node->RegisterWithGameInstance(world->GetGameInstance());

    TSharedPtr<FRootMotionSource_JumpForce> source = MakeShared<FRootMotionSource_JumpForce>();
    if (!source.IsValid()) return nullptr;

    source->InstanceName = FName("JumpForce");
    source->AccumulateMode = ERootMotionAccumulateMode::Override;
    source->Priority = 5;
    source->Rotation = Direction.Rotation();
    source->Distance = Distance;
    source->Height = Height;
    source->Duration = Duration;
    source->FinishVelocityParams.Mode = VelocityOnFinishMode;
    source->FinishVelocityParams.SetVelocity = VelocityOnFinish;
    source->FinishVelocityParams.ClampVelocity = ClampVelocityOnFinish;

    Node->pendingSource = source;

    return Node;
}

/* ---------------- MOVE TO ---------------- */

UAsyncRootMovement* UAsyncRootMovement::AsyncRootMovement_MoveTo(
    const UObject* WorldContext,
    UCharacterMovementComponent* InCharacterMovement,
    FVector StartLocation,
    FVector TargetLocation,
    float Duration,
    bool bRestrictSpeedToExpected,
    ERootMotionFinishVelocityMode VelocityOnFinishMode,
    FVector VelocityOnFinish,
    float ClampVelocityOnFinish)
{
    UWorld* world = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull);
    if (!world) return nullptr;

    UAsyncRootMovement* Node = NewObject<UAsyncRootMovement>();
    Node->world = world;
    Node->charMoveComp = InCharacterMovement;
    Node->RegisterWithGameInstance(world->GetGameInstance());

    TSharedPtr<FRootMotionSource_MoveToForce> source = MakeShared<FRootMotionSource_MoveToForce>();
    if (!source.IsValid()) return nullptr;

    source->InstanceName = FName("MoveTo");
    source->AccumulateMode = ERootMotionAccumulateMode::Override;
    source->Priority = 5;
    source->StartLocation = StartLocation;
    source->TargetLocation = TargetLocation;
    source->Duration = Duration;
    source->bRestrictSpeedToExpected = bRestrictSpeedToExpected;
    source->FinishVelocityParams.Mode = VelocityOnFinishMode;
    source->FinishVelocityParams.SetVelocity = VelocityOnFinish;
    source->FinishVelocityParams.ClampVelocity = ClampVelocityOnFinish;

    Node->pendingSource = source;

    return Node;
}

UAsyncRootMovement* UAsyncRootMovement::AsyncRootMovement_MoveToDynamic(
    const UObject* WorldContext,
    UCharacterMovementComponent* InCharacterMovement,
    FVector StartLocation,
    FVector InitialTargetLocation,
    float Duration,
    bool bRestrictSpeedToExpected,
    ERootMotionFinishVelocityMode VelocityOnFinishMode,
    FVector VelocityOnFinish,
    float ClampVelocityOnFinish)
{
    UWorld* world = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull);
    if (!world) return nullptr;

    UAsyncRootMovement* Node = NewObject<UAsyncRootMovement>();
    Node->world = world;
    Node->charMoveComp = InCharacterMovement;
    Node->RegisterWithGameInstance(world->GetGameInstance());

    TSharedPtr<FRootMotionSource_MoveToDynamicForce> source = MakeShared<FRootMotionSource_MoveToDynamicForce>();
    if (!source.IsValid()) return nullptr;

    source->InstanceName = FName("MoveToDynamic");
    source->AccumulateMode = ERootMotionAccumulateMode::Override;
    source->Priority = 5;
    source->StartLocation = StartLocation;
    source->InitialTargetLocation = InitialTargetLocation;
    source->Duration = Duration;
    source->bRestrictSpeedToExpected = bRestrictSpeedToExpected;
    source->FinishVelocityParams.Mode = VelocityOnFinishMode;
    source->FinishVelocityParams.SetVelocity = VelocityOnFinish;
    source->FinishVelocityParams.ClampVelocity = ClampVelocityOnFinish;

    Node->pendingSource = source;

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
    UWorld* world = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull);
    if (!world) return nullptr;

    UAsyncRootMovement* Node = NewObject<UAsyncRootMovement>();
    Node->world = world;
    Node->charMoveComp = InCharacterMovement;
    Node->RegisterWithGameInstance(world->GetGameInstance());

    TSharedPtr<FRootMotionSource_RadialForce> source = MakeShared<FRootMotionSource_RadialForce>();
    if (!source.IsValid()) return nullptr;
    
    source->InstanceName = FName("RadialForce");
    source->AccumulateMode = ERootMotionAccumulateMode::Additive;
    source->Priority = 5;
    source->Location = Origin;
    source->Radius = Radius;
    source->Strength = Strength;
    source->Duration = Duration;
    source->bIsPush = bIsPush;
    source->StrengthOverTime = StrengthOverTime;

    Node->pendingSource = source;

    return Node;
}

/* ---------------- ACTIVATE ---------------- */

void UAsyncRootMovement::Activate()
{
    ApplyRootMotion();
    pendingSource = nullptr;
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

        if (!charMoveComp.IsValid()) OnInterrupted.Broadcast(this); // Invalid Character movement = Interrupted
        else
        {
            UCharacterMovementComponent* moveComp = charMoveComp.Get();
            if (moveComp->GetRootMotionSourceByID(rootMotionSourceID).IsValid()) // Valid + Canceling = Interrupted. Root Motion still happening, but we're canceling it
            {
                moveComp->RemoveRootMotionSourceByID(rootMotionSourceID);
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
    if (!World || !charMoveComp.IsValid() || !pendingSource.IsValid()) Cancel();
    else
    {
        UCharacterMovementComponent* moveComp = charMoveComp.Get();
        rootMotionSourceID = moveComp->ApplyRootMotionSource(pendingSource);
        World->GetTimerManager().SetTimer(TH_OnGoing, this, &UAsyncRootMovement::CheckRootMotionStatus, 0.02f, true);
    }
}

void UAsyncRootMovement::CheckRootMotionStatus()
{
    if (bWasCancelled) return;
    else if (!charMoveComp.IsValid()) Cancel();
    else
    {
        UCharacterMovementComponent* moveComp = charMoveComp.Get();
        TSharedPtr<FRootMotionSource> rms = moveComp->GetRootMotionSourceByID(rootMotionSourceID);
        if (!rms.IsValid()) Cancel();
    }
}

void UAsyncRootMovement::UpdateMoveToDynamicTargetLocation(FVector NewLoc)
{
    if (!IsInGameThread() || !charMoveComp.IsValid()) return;
    UCharacterMovementComponent* moveComp = charMoveComp.Get();

    TSharedPtr<FRootMotionSource> rms = moveComp->GetRootMotionSourceByID(rootMotionSourceID);
    if (!rms.IsValid()) return;

    if (rms->GetScriptStruct() != FRootMotionSource_MoveToDynamicForce::StaticStruct()) return;

    TSharedPtr<FRootMotionSource_MoveToDynamicForce> MoveToDynamicRMS = StaticCastSharedPtr<FRootMotionSource_MoveToDynamicForce>(rms);
    if (!MoveToDynamicRMS.IsValid()) return;

    MoveToDynamicRMS->SetTargetLocation(NewLoc);
}