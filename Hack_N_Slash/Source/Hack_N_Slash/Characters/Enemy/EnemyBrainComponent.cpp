#include "EnemyBrainComponent.h"
#include "Modules/EnemyBrainModule.h"
#include "../../Controllers/EnemyController.h"
#include "../StateMachineComponent.h"

UEnemyBrainComponent::UEnemyBrainComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UEnemyBrainComponent::BeginPlay()
{
    Super::BeginPlay();

    //Wait for state machine to initialize states
    GetWorld()->GetTimerManager().SetTimer(TH_Wait,this, &UEnemyBrainComponent::Wait, 0.5f, false);
}

void UEnemyBrainComponent::Wait()
{
    CachePointers();
    InitializeModules();

    if (controller)
    {
        forgetSeenActorGracePeriod = controller->GetMaxAgeSight();
        controller->OnSensedSightDel.AddDynamic(this, &UEnemyBrainComponent::HandleSensedSight);
        controller->OnLostSightDel.AddDynamic(this, &UEnemyBrainComponent::HandleLostSight);
        controller->OnSensedSoundDel.AddDynamic(this, &UEnemyBrainComponent::HandleSensedSound);
        controller->OnEQSQueryFinishedDel.AddDynamic(this, &UEnemyBrainComponent::HandleEQSQueryFinished);
        controller->OnMoveCompletedDel.AddDynamic(this, &UEnemyBrainComponent::HandleMoveCompleted);
    }

    if (AActor* owner = GetOwner()) blackboard.HomeLocation = owner->GetActorLocation();

    UWorld* world = GetWorld();
    if (!world) return;

    world->GetTimerManager().SetTimer(TH_Decision, this, &UEnemyBrainComponent::DecisionTick, decisionInterval, true);
    if (bActive) RequestReevaluate();
}

void UEnemyBrainComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (activeModule) DeactivateModule(activeModule);

    moduleInstances.Empty();

    if (controller)
    {
        controller->OnSensedSightDel.RemoveDynamic(this, &UEnemyBrainComponent::HandleSensedSight);
        controller->OnLostSightDel.RemoveDynamic(this, &UEnemyBrainComponent::HandleLostSight);
        controller->OnSensedSoundDel.RemoveDynamic(this, &UEnemyBrainComponent::HandleSensedSound);
        controller->OnEQSQueryFinishedDel.RemoveDynamic(this, &UEnemyBrainComponent::HandleEQSQueryFinished);
        controller->OnMoveCompletedDel.RemoveDynamic(this, &UEnemyBrainComponent::HandleMoveCompleted);
    }

    UWorld* world = GetWorld();
    if (!world) return;

    world->GetTimerManager().ClearAllTimersForObject(this);
    Super::EndPlay(EndPlayReason);
}

void UEnemyBrainComponent::ActivateBrain()
{
    UWorld* world = GetWorld();
    if (!world) return;

    bActive = true;
    world->GetTimerManager().SetTimer(TH_Decision, this, &UEnemyBrainComponent::DecisionTick, decisionInterval, true);

}

void UEnemyBrainComponent::DeactivateBrain()
{
    UWorld* world = GetWorld();
    if (!world) return;

    bActive = false;
    world->GetTimerManager().ClearTimer(TH_Decision);
}

void UEnemyBrainComponent::CachePointers()
{
    APawn* PawnOwner = Cast<APawn>(GetOwner());

    if (!controller && PawnOwner) controller = Cast<AEnemyController>(PawnOwner->GetController());
    if (!stateMachineComp && PawnOwner) stateMachineComp = PawnOwner->FindComponentByClass<UStateMachineComponent>();
}

void UEnemyBrainComponent::InitializeModules()
{
    moduleInstances.Empty();

    for (auto& Cls : moduleClasses)
    {
        if (!Cls) continue;

        UEnemyBrainModule* Inst = NewObject<UEnemyBrainModule>(this, Cls);
        if (!Inst) continue;

        Inst->Initialize(this);
        moduleInstances.Add(Inst);
    }
    moduleInstances.Sort([](const UEnemyBrainModule& A, const UEnemyBrainModule& B) { return static_cast<int>(A.priority) > static_cast<int>(B.priority); });
}

void UEnemyBrainComponent::DecisionTick()
{
    if (!bActive || !bReevaluationRequested) return;
    bReevaluationRequested = false;
    EvaluateModules(TEXT("DecisionTick"));
}

void UEnemyBrainComponent::RequestReevaluate() { bReevaluationRequested = true; }

void UEnemyBrainComponent::EvaluateModules(const FString& Reason)
{
    if (bEvaluating) return;
    bEvaluating = true;

    for (UEnemyBrainModule* M : moduleInstances)
    {
        if (!M || !M->CanStart(Reason) || activeModule == M) continue;

        if (activeModule && activeModule->moduleState == EBrainState::Active && !activeModule->CanBeInterruptedBy(M)) continue;

        if (activeModule) DeactivateModule(activeModule);

        ActivateModule(M);
        break;
    }

    bEvaluating = false;
}

void UEnemyBrainComponent::ActivateModule(UEnemyBrainModule* Module)
{
    if (!Module) return;

    activeModule = Module;
    activeModule->OnEnter();
}

void UEnemyBrainComponent::DeactivateModule(UEnemyBrainModule* Module)
{
    if (!Module) return;
    Module->OnExit();
    if (activeModule == Module) activeModule = nullptr;
}

void UEnemyBrainComponent::HandleSensedSight(AActor* Seen)
{
    if (!bActive || !Seen) return;

    UWorld* world = GetWorld();
    if (!world) return;

    world->GetTimerManager().ClearTimer(TH_ForgetTarget);

    blackboard.TargetActor = Seen;
    blackboard.LastKnownLocation = Seen->GetActorLocation();

    if (activeModule)
    {
        activeModule->HandleSensedSight(Seen);
        // Design decison that nothing interrupts an enemy returning except death
        // Once they return they'll "reset"
        if (activeModule->moduleName == "Return") return;
    }
    RequestReevaluate();
}

void UEnemyBrainComponent::HandleLostSight(AActor* Lost)
{
    if (!bActive || !blackboard.TargetActor || (activeModule && activeModule->moduleName == "Return")) return;

    if (blackboard.TargetActor == Lost)
    {
        blackboard.LastKnownLocation = Lost->GetActorLocation();

        UWorld* world = GetWorld();
        if (world) world->GetTimerManager().SetTimer(TH_ForgetTarget, this, &UEnemyBrainComponent::HandleForgetSeenTarget, forgetSeenActorGracePeriod, false);
    }

    if (activeModule) activeModule->HandleLostSight(Lost);
    RequestReevaluate();
}

void UEnemyBrainComponent::HandleForgetSeenTarget()
{
    if (!bActive || !blackboard.TargetActor) return;

    AActor* forgotActor = blackboard.TargetActor;

    blackboard.TargetActor = nullptr;
    blackboard.LastKnownLocation = FVector::ZeroVector;
    blackboard.bForgotTarget = true;

    if (activeModule) activeModule->HandleForgetSeenTarget(forgotActor);
    RequestReevaluate();
}

void UEnemyBrainComponent::HandleSensedSound(AActor* Heard, const FVector& Origin)
{
    if (!bActive) return;
    if (!blackboard.TargetActor) blackboard.LastKnownLocation = Origin;
    if (activeModule) activeModule->HandleSensedSound(Heard, Origin);
    RequestReevaluate();
}

void UEnemyBrainComponent::HandleEQSQueryFinished(const FEnvQueryResult& Result)
{
    Result.GetAllAsActors(blackboard.EQS_Actors);
    Result.GetAllAsLocations(blackboard.EQS_Locs);

    if (!bActive) return;
    if (activeModule) activeModule->HandleEQSFinished(Result);
    RequestReevaluate();
}

void UEnemyBrainComponent::HandleMoveCompleted(bool bSuccess)
{
    if (!bActive) return;
    if (activeModule) activeModule->HandleMoveCompleted(bSuccess);
    RequestReevaluate();
}

void UEnemyBrainComponent::HandleAnimNotify(FName NotifyName)
{
    if (!bActive || !controller) return;
    if (activeModule) activeModule->HandleAnimNotify(NotifyName);
    RequestReevaluate();
}

void UEnemyBrainComponent::HandleReceiveHit(const FAtkHitData& HitData)
{
    if (!bActive || !controller) return;
    blackboard.LastDamageSource = HitData.attacker;
    if (activeModule) activeModule->HandleReceiveHit(HitData);
    RequestReevaluate();
}