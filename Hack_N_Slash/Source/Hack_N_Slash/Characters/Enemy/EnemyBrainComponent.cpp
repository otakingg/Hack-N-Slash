#include "EnemyBrainComponent.h"
#include "Modules/EnemyBrainModule.h"
#include "Controllers/EnemyController.h"
#include "../StateMachineComponent.h"

UEnemyBrainComponent::UEnemyBrainComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UEnemyBrainComponent::BeginPlay()
{
    Super::BeginPlay();

    CachePointers();
    InitializeModules();

    if (controller)
    {
        forgetSeenActorGracePeriod = controller->GetMaxAgeSight();
        controller->OnSensedSightDel.AddDynamic(this, &UEnemyBrainComponent::HandleSensedSight);
        controller->OnLostSightDel.AddDynamic(this, &UEnemyBrainComponent::HandleLostSight);
        controller->OnSensedDamageDel.AddDynamic(this, &UEnemyBrainComponent::HandleSensedDamage);
        controller->OnSensedSoundDel.AddDynamic(this, &UEnemyBrainComponent::HandleSensedSound);
        controller->OnEQSQueryFinishedDel.AddDynamic(this, &UEnemyBrainComponent::HandleEQSQueryFinished);
        controller->OnMoveCompletedDel.AddDynamic(this, &UEnemyBrainComponent::HandleMoveCompleted);
    }

    if (reevaluateIntervalSeconds > 0.f) GetWorld()->GetTimerManager().SetTimer(TH_Reeval, this, &UEnemyBrainComponent::RequestReevaluate, reevaluateIntervalSeconds, true);
    EvaluateModules(TEXT("Begin Play"));
}

void UEnemyBrainComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (controller)
    {
        controller->OnSensedSightDel.RemoveDynamic(this, &UEnemyBrainComponent::HandleSensedSight);
        controller->OnLostSightDel.RemoveDynamic(this, &UEnemyBrainComponent::HandleLostSight);
        controller->OnSensedDamageDel.RemoveDynamic(this, &UEnemyBrainComponent::HandleSensedDamage);
        controller->OnSensedSoundDel.RemoveDynamic(this, &UEnemyBrainComponent::HandleSensedSound);
        controller->OnEQSQueryFinishedDel.RemoveDynamic(this, &UEnemyBrainComponent::HandleEQSQueryFinished);
        controller->OnMoveCompletedDel.RemoveDynamic(this, &UEnemyBrainComponent::HandleMoveCompleted);
    }

    GetWorld()->GetTimerManager().ClearTimer(TH_Reeval);
    GetWorld()->GetTimerManager().ClearTimer(TH_ForgetTarget);
    Super::EndPlay(EndPlayReason);
}

void UEnemyBrainComponent::CachePointers()
{
    if (!controller)
    {
        APawn* PawnOwner = Cast<APawn>(GetOwner());
        controller = PawnOwner ? Cast<AEnemyController>(PawnOwner->GetController()) : nullptr;
    }
    if (!stateMachineComp)
    {
        APawn* PawnOwner = Cast<APawn>(GetOwner());
        stateMachineComp = PawnOwner ? PawnOwner->FindComponentByClass<UStateMachineComponent>() : nullptr;
    }
}

void UEnemyBrainComponent::InitializeModules()
{
    moduleInstances.Empty();

    for (const TSubclassOf<UEnemyBrainModule>& Cls : moduleClasses)
    {
        if (!Cls) continue;

        UEnemyBrainModule* Inst = NewObject<UEnemyBrainModule>(this, Cls.Get());
        if (!Inst) continue;

        Inst->Initialize(this);
        moduleInstances.Add(Inst);
    }

    // Remove any nulls just in case, then sort by priority descending
    moduleInstances.RemoveAll([](UEnemyBrainModule* M) { return M == nullptr; });
    moduleInstances.Sort([](const UEnemyBrainModule& A, const UEnemyBrainModule& B) { return static_cast<int>(A.priority) > static_cast<int>(B.priority); });
}

void UEnemyBrainComponent::EvaluateModules(const FString& Reason)
{
    for (UEnemyBrainModule* M : moduleInstances)
    {
        if (!M || !M->CanStart(Reason)) continue;

        if (activeModule)
        {
            if (activeModule->CanBeInterruptedBy(M)) {DeactivateModule(activeModule);}
            else continue;
        }
        ActivateModule(M); return;
    }
}

void UEnemyBrainComponent::ActivateModule(UEnemyBrainModule* Module)
{
    if (!Module) return;
    if (activeModule) DeactivateModule(activeModule);
    activeModule = Module;
    activeModule->OnEnter();
}
void UEnemyBrainComponent::DeactivateModule(UEnemyBrainModule* Module)
{
    if (!Module) return;
    Module->OnExit();
    if (activeModule == Module) activeModule = nullptr;
}

void UEnemyBrainComponent::RequestReevaluate() { EvaluateModules(TEXT("Reevaluate")); }

/* ---------------- Event Handlers ---------------- */
void UEnemyBrainComponent::HandleSensedSight(AActor* Seen)
{
    if (!Seen) return;

    // Cancel any pending forget timer if our current (or any) target is sensed again
    UWorld* world = {GetWorld()};
    if (world) world->GetTimerManager().ClearTimer(TH_ForgetTarget);

    blackboard.TargetActor = Seen;
    blackboard.LastKnownLocation = Seen->GetActorLocation();

    if (activeModule) activeModule->HandleSensedSight(Seen);
    for (UEnemyBrainModule* M : moduleInstances) if (M && M != activeModule) M->HandleSensedSight(Seen);
    EvaluateModules(TEXT("Sight"));
}

void UEnemyBrainComponent::HandleLostSight(AActor* Lost)
{
    // If the actor we lost sight of is our current target, start a forget timer.
    if (!blackboard.TargetActor) return;

    if (blackboard.TargetActor == Lost)
    {
        blackboard.LastKnownLocation = blackboard.TargetActor->GetActorLocation();
        UWorld* world = {GetWorld()};
        if (world)
        {
            world->GetTimerManager().ClearTimer(TH_ForgetTarget);
            world->GetTimerManager().SetTimer(TH_ForgetTarget, this, &UEnemyBrainComponent::HandleForgetSeenTarget, forgetSeenActorGracePeriod, false);
        }
    }

    // keep existing module calls
    if (activeModule) activeModule->HandleLostSight(Lost);
    for (UEnemyBrainModule* M : moduleInstances) if (M && M != activeModule) M->HandleLostSight(Lost);
    EvaluateModules(TEXT("LostSight"));
}

void UEnemyBrainComponent::HandleForgetSeenTarget()
{
    if (!blackboard.TargetActor) return;
    
    if (!controller) CachePointers();
    if (controller && controller->IsActorSeen(blackboard.TargetActor))
    {
        // If controller reports actor currently seen, cancel clearing
        GetWorld()->GetTimerManager().ClearTimer(TH_ForgetTarget);
        return;
    }

    AActor* forgotActor = blackboard.TargetActor;
    if (activeModule)
    {
        blackboard.TargetActor = nullptr;
        blackboard.LastKnownLocation = FVector::ZeroVector;
        activeModule->HandleForgetSeenTarget(forgotActor);
    }
    else
    {
        blackboard.TargetActor = nullptr;
        blackboard.LastKnownLocation = FVector::ZeroVector;
    }

    // Let modules know we forgot target and reevaluate
    for (UEnemyBrainModule* M : moduleInstances) if (M && M != activeModule) M->HandleForgetSeenTarget(forgotActor);
    EvaluateModules(TEXT("ForgotTarget"));
}

void UEnemyBrainComponent::HandleSensedDamage(AActor* Source)
{
    blackboard.LastDamageSource = Source;
    if (activeModule) activeModule->HandleSensedDamage(Source);
    for (UEnemyBrainModule* M : moduleInstances) if (M && M != activeModule) M->HandleSensedDamage(Source);
    EvaluateModules(TEXT("Damage"));
}

void UEnemyBrainComponent::HandleSensedSound(AActor* Heard, const FVector& Origin)
{
    if (!blackboard.TargetActor) blackboard.LastKnownLocation = Origin;
    if (activeModule) activeModule->HandleSensedSound(Heard, Origin);
    for (UEnemyBrainModule* M : moduleInstances) if (M && M != activeModule) M->HandleSensedSound(Heard, Origin);
    EvaluateModules(TEXT("Sound"));
}

void UEnemyBrainComponent::HandleEQSQueryFinished(const FEnvQueryResult& Result)
{
    Result.GetAllAsActors(blackboard.EQS_Actors);
    Result.GetAllAsLocations(blackboard.EQS_Locs);
    if (activeModule) activeModule->HandleEQSFinished(Result);
    for (UEnemyBrainModule* M : moduleInstances) if (M && M != activeModule) M->HandleEQSFinished(Result);
    EvaluateModules(TEXT("EQS"));
}

void UEnemyBrainComponent::HandleMoveCompleted(bool bSuccess)
{
    if (activeModule) activeModule->HandleMoveCompleted(bSuccess);
    for (UEnemyBrainModule* M : moduleInstances) if (M && M != activeModule) M->HandleMoveCompleted(bSuccess);
    EvaluateModules(TEXT("MoveCompleted"));
}

void UEnemyBrainComponent::HandleAnimNotify(FName NotifyName)
{
    if (activeModule) activeModule->HandleAnimNotify(NotifyName);
    for (UEnemyBrainModule* M : moduleInstances) if (M && M != activeModule) M->HandleAnimNotify(NotifyName);
    EvaluateModules(TEXT("AnimNotify"));
}