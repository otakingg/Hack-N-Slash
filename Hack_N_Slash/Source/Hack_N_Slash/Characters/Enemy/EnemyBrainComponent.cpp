#include "EnemyBrainComponent.h"
#include "../../Tags/CharacterStateTagNamespaces.h"
#include "Modules/EnemyBrainModule.h"
#include "../../Combat/Enemy/EnemyCombatComponent.h"
#include "../../Controllers/EnemyController.h"
#include "Sequences/EnemySequence.h"
#include "../Shared/LocomotionComponent.h"
#include "../Shared/StateMachineComponent.h"

UEnemyBrainComponent::UEnemyBrainComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UEnemyBrainComponent::BeginPlay()
{
    Super::BeginPlay();

    SetComponentTickEnabled(false);
    activeAggroDecayRate = aggroDecayRateLostSight;

    //Wait for state machine to initialize states
    if (UWorld* world = GetWorld()) world->GetTimerManager().SetTimer(TH_Wait,this, &UEnemyBrainComponent::Wait, 0.5f, false);
}

void UEnemyBrainComponent::Wait()
{
    UWorld* world = GetWorld();
    if (!world) return;

    CachePointers();
    InitializeModulesAndSequences();

    if (controller)
    {
        forgetSeenActorGracePeriod = controller->GetMaxAgeSight();

        controller->OnSensedSightDel.AddUObject(this, &UEnemyBrainComponent::HandleSensedSight);
        controller->OnLostSightDel.AddUObject(this, &UEnemyBrainComponent::HandleLostSight);
        controller->OnSensedSoundDel.AddUObject(this, &UEnemyBrainComponent::HandleSensedSound);
        controller->OnEQSQueryFinishedDel.AddUObject(this, &UEnemyBrainComponent::HandleEQSQueryFinished);
        controller->OnMoveCompletedDel.AddUObject(this, &UEnemyBrainComponent::HandleMoveCompleted);
    }

    if (AActor* owner = GetOwner()) blackboard.HomeLocation = owner->GetActorLocation();

    if (bActive)
    {
        world->GetTimerManager().SetTimer(TH_Decision, this, &UEnemyBrainComponent::DecisionTick, decisionInterval, true);
        RequestReevaluate();
    }
}

void UEnemyBrainComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    timeSinceLastAggro += DeltaTime;

    // Only decay after delay
    if (timeSinceLastAggro >= aggroDecayDelay)
    {
        blackboard.Aggro -= activeAggroDecayRate * DeltaTime;

        blackboard.Aggro = FMath::Clamp(blackboard.Aggro, 0.0f, aggroMax);
    }

    // Example AI behavior checks
    if (blackboard.Aggro <= 0.0f) SetComponentTickEnabled(false);

}

void UEnemyBrainComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    DeactivateModule();

    moduleInstances.Empty();

    if (controller)
    {
        controller->OnSensedSightDel.RemoveAll(this);
        controller->OnLostSightDel.RemoveAll(this);
        controller->OnSensedSoundDel.RemoveAll(this);
        controller->OnEQSQueryFinishedDel.RemoveAll(this);
        controller->OnMoveCompletedDel.RemoveAll(this);
    }

    if (UWorld* world = GetWorld()) world->GetTimerManager().ClearAllTimersForObject(this);
    
    Super::EndPlay(EndPlayReason);
}

void UEnemyBrainComponent::ActivateBrain()
{
    SetComponentTickEnabled(true);
    UWorld* world = GetWorld();
    if (!world) return;

    bActive = true;
    world->GetTimerManager().SetTimer(TH_Decision, this, &UEnemyBrainComponent::DecisionTick, decisionInterval, true);

}

void UEnemyBrainComponent::DeactivateBrain()
{
    SetComponentTickEnabled(false);
    UWorld* world = GetWorld();
    if (!world) return;

    world->GetTimerManager().ClearTimer(TH_Decision);
    bActive = false;
}

void UEnemyBrainComponent::CachePointers()
{
    APawn* PawnOwner = Cast<APawn>(GetOwner());

    if (!combatComp && PawnOwner) combatComp = PawnOwner->FindComponentByClass<UEnemyCombatComponent>();
    if (!controller && PawnOwner) controller = Cast<AEnemyController>(PawnOwner->GetController());
    if (!locoComp && PawnOwner) locoComp = PawnOwner->FindComponentByClass<ULocomotionComponent>();
    if (!stateMachineComp && PawnOwner) stateMachineComp = PawnOwner->FindComponentByClass<UStateMachineComponent>();
}

void UEnemyBrainComponent::InitializeModulesAndSequences()
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


    sequenceInstances.Empty();
    for (auto& Cls : sequenceClasses)
    {
        if (!Cls) continue;

        UEnemySequence* Inst = NewObject<UEnemySequence>(this, Cls);
        if (!Inst) continue;

        Inst->Initialize(this);
        sequenceInstances.Add(Inst);
    }
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
    if (bEvaluating || (activeModule && activeModule->moduleState == EBrainState::Exiting)) return;
    //if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("[EnemyBrainComp] Evaluating"));
    bEvaluating = true;

    for (UEnemyBrainModule* M : moduleInstances)
    {
        if (!M || !M->CanStart(Reason) || activeModule == M) continue;

        if (activeModule && activeModule->moduleState == EBrainState::Active && !activeModule->CanBeInterruptedBy(M)) continue;

        if (activeModule) DeactivateModule();

        ActivateModule(M);
        break;
    }

   if (blackboard.bStaggered) blackboard.bStaggered = false;
    bEvaluating = false;
}

void UEnemyBrainComponent::ActivateModule(UEnemyBrainModule* Module)
{
    if (!Module) return;

    activeModule = Module;
    activeModule->OnEnter();
}

void UEnemyBrainComponent::DeactivateModule()
{
    if (!activeModule) return;
    activeModule->OnExit();
    activeModule = nullptr;
}

void UEnemyBrainComponent::HandleSensedSight(AActor* Seen)
{
    if (!bActive || !Seen || blackboard.bForgotTarget) return;

    UWorld* world = GetWorld();
    if (!world) return;

    world->GetTimerManager().ClearTimer(TH_ForgetTarget);

    activeAggroDecayRate = aggroDecayRateVisible;
    blackboard.TargetActor = Seen;
    blackboard.LastKnownLocation = Seen->GetActorLocation();

    if (activeModule) activeModule->HandleSensedSight(Seen);
    RequestReevaluate();
}

void UEnemyBrainComponent::HandleLostSight(AActor* Lost)
{
    if (!bActive || !blackboard.TargetActor || blackboard.bForgotTarget) return;

    if (blackboard.TargetActor == Lost)
    {
        activeAggroDecayRate = aggroDecayRateLostSight;
        blackboard.LastKnownLocation = Lost->GetActorLocation();

        UWorld* world = GetWorld();
        if (world) world->GetTimerManager().SetTimer(TH_ForgetTarget, this, &UEnemyBrainComponent::HandleForgetSeenTarget, forgetSeenActorGracePeriod, false);
    }

    if (activeModule) activeModule->HandleLostSight(Lost);
    RequestReevaluate();
}

void UEnemyBrainComponent::HandleForgetSeenTarget()
{
    if (!bActive || !blackboard.TargetActor || !controller) return;

    blackboard.Aggro = 0.0f;
    if (activeModule) activeModule->HandleForgetSeenTarget(blackboard.TargetActor);
    RequestReevaluate();
}

void UEnemyBrainComponent::HandleSensedSound(AActor* Heard, const FVector& Origin)
{
    if (!bActive || blackboard.bForgotTarget) return;
    if (!blackboard.TargetActor) blackboard.LastKnownLocation = Origin;
    if (activeModule) activeModule->HandleSensedSound(Heard, Origin);
    RequestReevaluate();
}

void UEnemyBrainComponent::HandleEQSQueryFinished(const FEnvQueryResult& Result)
{
    if (!bActive) return;

    blackboard.EQS_Actors.Empty();
    blackboard.EQS_Locs.Empty();

    Result.GetAllAsActors(blackboard.EQS_Actors);
    Result.GetAllAsLocations(blackboard.EQS_Locs);

    if (activeModule) activeModule->HandleEQSFinished(Result);
    RequestReevaluate();
}

void UEnemyBrainComponent::HandleMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
    if (!bActive) return;
    if (activeModule) activeModule->HandleMoveCompleted(RequestID.GetID(), Result);
    RequestReevaluate();
}

void UEnemyBrainComponent::HandleAnimNotify(FGameplayTag NotifyTag)
{
    if (!bActive || !controller) return;
    if (activeModule) activeModule->HandleAnimNotify(NotifyTag);
    RequestReevaluate();
}

void UEnemyBrainComponent::HandleMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
    if (!bActive || !controller) return;
    if (activeModule) activeModule->HandleMontageBlendingOut(Montage, bInterrupted);
    RequestReevaluate();
}

void UEnemyBrainComponent::HandleReceiveHitPre(FAtkHitData& HitData)
{
    if (!bActive || !controller || blackboard.bForgotTarget) return;
    if (activeModule) activeModule->HandleReceiveHitPre(HitData);
    RequestReevaluate();
}

void UEnemyBrainComponent::HandleReceiveHitPost(FAtkHitData& HitData)
{
    if (!bActive || !controller || blackboard.bForgotTarget) return;
    blackboard.LastDamageSource = HitData.attacker;
    if (HitData.resolvedReaction != ActionTags::None) blackboard.bStaggered = true;
    if (HitData.dmgHPDealt > 0.0f)
    {
        blackboard.Aggro += HitData.aggroBuildup;
        if (!IsComponentTickEnabled()) SetComponentTickEnabled(true);
    }
    if (activeModule) activeModule->HandleReceiveHitPost(HitData);
    RequestReevaluate();
}

void UEnemyBrainComponent::HandleAttackDetected()
{
    if (!bActive || !controller || blackboard.bForgotTarget) return;
    if (activeModule) activeModule->HandleAttackDetected();
    RequestReevaluate();
}