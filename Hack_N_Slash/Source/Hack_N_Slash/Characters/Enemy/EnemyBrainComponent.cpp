#include "EnemyBrainComponent.h"
#include "../../Interfaces/CombatInstigator.h"
#include "../../Controllers/EnemyController.h"
#include "Sequences/EnemSeqProactive.h"
#include "Sequences/EnemSeqReactive.h"
#include "../../Structs/FAtkHitData.h"
#include "../Shared/StateMachineComponent.h"
#include "../../Utility/Tags.h"

UEnemyBrainComponent::UEnemyBrainComponent() { PrimaryComponentTick.bCanEverTick = true; }

void UEnemyBrainComponent::BeginPlay()
{
    Super::BeginPlay();

    SetComponentTickEnabled(false);

    if (!EnsureReferences()) return;

    InitializeSequences();

    blackboard.HomeLocation = owner->GetActorLocation();

    if (controller)
    {
        forgetSeenActorGracePeriod = controller->GetMaxAgeSight();

        controller->OnSensedSight.AddUObject(this, &UEnemyBrainComponent::HandleSensedSight);
        controller->OnLostSight.AddUObject(this, &UEnemyBrainComponent::HandleLostSight);
        controller->OnEQSQueryFinishedDel.AddUObject(this, &UEnemyBrainComponent::HandleEQSQueryFinished);
        controller->OnMoveCompletedDel.AddUObject(this, &UEnemyBrainComponent::HandleMoveCompleted);
    }

    DecisionTick();
    if (UWorld* world = GetWorld()) world->GetTimerManager().SetTimer(TH_Decision, this, &UEnemyBrainComponent::DecisionTick, decisionInterval, true);

}

void UEnemyBrainComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (blackboard.RecentHits <= 0) SetComponentTickEnabled(false);
    else if (UWorld* world = GetWorld())
    {
        if (lastHitTime < 0 || world->GetTimeSeconds() - lastHitTime >= recentHitsResetDelay) blackboard.RecentHits = 0;
    }
}

void UEnemyBrainComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    DeactivateSequence();

    proactiveSequenceInstances.Empty();
    reactiveSequenceInstances.Empty();

    if (controller)
    {
        controller->OnSensedSight.RemoveAll(this);
        controller->OnLostSight.RemoveAll(this);
        controller->OnEQSQueryFinishedDel.RemoveAll(this);
        controller->OnMoveCompletedDel.RemoveAll(this);
    }

    if (UWorld* world = GetWorld()) world->GetTimerManager().ClearAllTimersForObject(this);
    
    Super::EndPlay(EndPlayReason);
}

bool UEnemyBrainComponent::EnsureReferences()
{
    if (!owner) owner = Cast<APawn>(GetOwner());
    if (!owner)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UEnemyBrainComponent] Owner is not an APawn: %s"), *GetNameSafe(GetOwner()));
        return false;
    }

    if (!iCmbtInst) iCmbtInst = Cast<ICombatInstigator>(owner);
    if (!iCmbtInst)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UEnemyBrainComponent] Owner isn't an ICombatInstigator: %s"), *GetNameSafe(owner));
        return false;
    }

    if (!controller) controller = Cast<AEnemyController>(owner->GetController());
    if (!controller)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UEnemyBrainComponent] Owner has no AEnemyController: %s"), *GetNameSafe(owner));
        return false;
    }

    if (!smComp) smComp = owner->FindComponentByClass<UStateMachineComponent>();
    if (!smComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UEnemyBrainComponent] Owner has no UStateMachineComponent: %s"), *GetNameSafe(owner));
        return false;
    }

    return true;
}

void UEnemyBrainComponent::PauseBrain()
{
    bActive = false;

    if (UWorld* world = GetWorld())
    {
        FTimerManager& timerManager = world->GetTimerManager();
        timerManager.PauseTimer(TH_Decision);
        if (timerManager.IsTimerActive(TH_ForgetTarget)) timerManager.PauseTimer(TH_ForgetTarget);
    }
}

void UEnemyBrainComponent::UnpauseBrain()
{
    bActive = true;

    if (UWorld* world = GetWorld())
    {
        FTimerManager& timerManager = world->GetTimerManager();
        timerManager.UnPauseTimer(TH_Decision);
        if (timerManager.IsTimerPaused(TH_ForgetTarget)) timerManager.UnPauseTimer(TH_ForgetTarget);
    }
}

void UEnemyBrainComponent::ResetBrain()
{
    if (UWorld* world = GetWorld()) world->GetTimerManager().ClearAllTimersForObject(this);

    DeactivateSequence();
    SetComponentTickEnabled(false);
    if (controller) controller->ClearFocusHNS();
    blackboard.bForgotTarget = false;
    blackboard.EQS_Actors.Empty();
    blackboard.EQS_Locs.Empty();
    blackboard.TargetActor = nullptr;
    blackboard.TargetDistance = -1.0f;
    blackboard.TargetHeightDifference = 0.0f;
    blackboard.LastAttackTime = -1.0f;
    blackboard.LastDamageSource = nullptr;
    blackboard.LastKnownLocation = FVector::ZeroVector;
    blackboard.RecentHits = 0;
    lastHitTime = -1.0f;
}

void UEnemyBrainComponent::InitializeSequences()
{
    if (proactiveSequenceInstances.IsEmpty())
    {
        for (auto& cls : proactiveSequenceClasses)
        {
            if (!cls) continue;

            UEnemSeqProactive* Inst = NewObject<UEnemSeqProactive>(this, cls);
            if (!Inst) continue;

            Inst->Initialize(this);
            proactiveSequenceInstances.Add(Inst);
        }
    }

    if (reactiveSequenceInstances.IsEmpty())
    {
        for (auto& cls : reactiveSequenceClasses)
        {
            if (!cls) continue;

            UEnemSeqReactive* Inst = NewObject<UEnemSeqReactive>(this, cls);
            if (!Inst) continue;

            Inst->Initialize(this);
            reactiveSequenceInstances.Add(Inst);
        }
    }
}

void UEnemyBrainComponent::DecisionTick()
{
    if (!EnsureReferences() || !bActive || (activeSequence && !activeSequence->IsInterruptibleProactive())) return;

    CalculateTargetDistance(); // Update target distance
    EvaluateSequencesProactive(); // Evaluate proactive sequences
}

void UEnemyBrainComponent::CalculateTargetDistance()
{
    if (!blackboard.TargetActor)
    {
        blackboard.TargetDistance = -1.0f;
        blackboard.TargetHeightDifference = 0.0f;
        return;
    }

    FVector startLoc = owner->GetActorLocation();
    FVector endLoc = blackboard.TargetActor->GetActorLocation();

    blackboard.TargetDistance = FVector::Dist(startLoc, endLoc);
    blackboard.TargetHeightDifference = (endLoc - startLoc).Z;
}

void UEnemyBrainComponent::EvaluateSequencesProactive()
{
    if (bEvaluatingProactive) return; // Already evaluating, so leave

    bEvaluatingProactive = true;

    if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("[EnemyBrainComp] Evaluating"));

    UEnemSeqProactive* chosenSequence = nullptr;
    chosenSequence = GetBestSequenceOffCoolDownProactive(); // If there's a sequence that wants to be played off cooldown ignoring scores, choose it
    if (!chosenSequence) chosenSequence = GetBestSequenceProactive(); // Else pick the best scored sequence

    if (!chosenSequence)
    {
        if (bDebug)
        {
            if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("[EnemyBrainComp] No valid proactive sequence found"));
            UE_LOG(LogTemp, Warning, TEXT("[EnemyBrainComp] No valid proactive sequence found"));
        }
        bEvaluatingProactive = false;
        return;
    }

    if (activeSequence) DeactivateSequence(); // If there's an active sequence, deactivate it
    bEvaluatingProactive = false;
    ActivateSequenceProactive(chosenSequence);
}

UEnemSeqProactive* UEnemyBrainComponent::GetBestSequenceOffCoolDownProactive() const
{
    UEnemSeqProactive* bestSequence = nullptr;
    float bestScore = -1.0f;

    for (UEnemSeqProactive* sequence : proactiveSequenceInstances)
    {
        if (sequence && sequence->bForceOffCooldown && sequence->CanExecute() && sequence->GetScore() > bestScore)
        {
            bestSequence = sequence;
            bestScore = sequence->GetScore();
        }
    }
    return bestSequence;
}

UEnemSeqProactive* UEnemyBrainComponent::GetBestSequenceProactive() const
{
    struct FSequenceCandidate
    {
        UEnemSeqProactive* sequence = nullptr;
        float score = 0.0f;
    };

    TArray<FSequenceCandidate> candidates;
    float bestScore = -1.0f;

    // -------------------------
    // 1. Gather scores
    // -------------------------
    for (UEnemSeqProactive* sequence : proactiveSequenceInstances)
    {
        if (!sequence || !sequence->CanExecute()) continue;

        float score = sequence->GetScore();
        if (score <= 0) continue;

        if (bDebug)
        {
            if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, FString::Printf(TEXT("Sequence: %s | Score: %.2f"), *sequence->GetName(), score));
            UE_LOG(LogTemp, Display, TEXT("Sequence: %s | Score: %.2f"), *sequence->GetName(), score);
        }

        candidates.Add({ sequence, score });
        bestScore = FMath::Max(bestScore, score);
    }

    if (candidates.IsEmpty()) return nullptr;

    // -------------------------
    // 2. Calculate minimum viable score based on the selection threshold
    // -------------------------

    const float minViableScore = bestScore * selectionThreshold; //EX: Best score = 10, threshold = 0.7. Minimum viable score = 10 * 0.7 = 70

    // -------------------------
    // 3. Uniform Randomness
    // EX: 0.9 and 0.7 have equal chance to be selected, assuming they both survive the threshold
    // -------------------------

    // Try at most 100 times to find a viable candidate
    // Accept 1st candidate that meets the minimum viable score
    for (int32 i = 0; i < 100; ++i)
    {
        const int32 index = FMath::RandRange(0, candidates.Num() - 1);

        if (candidates[index].score >= minViableScore) return candidates[index].sequence;
    }

    return nullptr;
}

UEnemSeqReactive* UEnemyBrainComponent::GetBestSequenceReactive(const FAtkHitData& HitData) const
{
    struct FReactionCandidate
    {
        UEnemSeqReactive* sequence = nullptr;
        float score = 0.0f;
    };

    TArray<FReactionCandidate> candidates;
    float bestScore = -1.0f;

    // -------------------------
    // 1. Gather scores
    // -------------------------
    for (UEnemSeqReactive* sequence : reactiveSequenceInstances)
    {
        if (!sequence || !sequence->CanExecute(HitData)) continue;

        float score = sequence->GetScore();
        if (score <= 0) continue;

        if (bDebug)
        {
            if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, FString::Printf(TEXT("Sequence: %s | Score: %.2f"), *sequence->GetName(), score));
            UE_LOG(LogTemp, Display, TEXT("Sequence: %s | Score: %.2f"), *sequence->GetName(), score);
        }

        candidates.Add({ sequence, score });
        bestScore = FMath::Max(bestScore, score);
    }

    if (candidates.IsEmpty()) return nullptr;

    // -------------------------
    // 2. Calculate minimum viable score based on a selection threshold AND total weight of viable candidates
    // -------------------------

    float totalWeight = 0.0f;

    for (FReactionCandidate& candidate : candidates) { totalWeight += candidate.score; }

    if (totalWeight <= KINDA_SMALL_NUMBER) return nullptr;

    // -------------------------
    // 3. Weighted Randomness
    // EX: 0.6 is twice as likely to win as 0.3, assuming both survive the threshold
    // -------------------------
    UEnemSeqReactive* chosenSequence = nullptr;
    float roll = FMath::FRandRange(0.0f, totalWeight);

    for (const FReactionCandidate& candidate : candidates)
    {
        roll -= candidate.score;
        if (roll <= 0.0f) // If the roll is <= 0, this candidate has been selected
        {
            if (candidate.sequence->RollChance()) chosenSequence = candidate.sequence; // If the candidate passes its roll chance, choose it
            break; // Break regardless of whether the candidate passes its personal roll chance or not
        }
    }

    return chosenSequence;
}

void UEnemyBrainComponent::ActivateSequenceProactive(UEnemSeqProactive* Sequence)
{
    if (!Sequence) return;

    if (prevSequenceName != Sequence->GetSeqName()) prevSequenceName = Sequence->GetSeqName();

    activeSequence = Sequence;
    Sequence->Execute();
}

void UEnemyBrainComponent::ActivateSequenceReactive(UEnemSeqReactive* Sequence, const FAtkHitData& HitData)
{
    if (!Sequence) return;

    if (prevSequenceName != Sequence->GetSeqName()) prevSequenceName = Sequence->GetSeqName();

    activeSequence = Sequence;
    Sequence->Execute(HitData);
}

void UEnemyBrainComponent::DeactivateSequence()
{
    if (!activeSequence) return;

    if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("[EnemyBrainComp] Interrupting Sequence"));

    UEnemySequence* oldSequence = activeSequence;
    activeSequence = nullptr;

    oldSequence->Abort();
}

void UEnemyBrainComponent::RemoveActiveSequence() { if (activeSequence) activeSequence = nullptr; }

void UEnemyBrainComponent::HandleSensedSight(AActor* Seen)
{
    if (!bActive || !EnsureReferences() || !Seen || blackboard.bForgotTarget) return;

    ICombatInstigator* iSeenCmbtInst = Cast<ICombatInstigator>(Seen);
    if (!iSeenCmbtInst || iSeenCmbtInst->GetTeamID() == iCmbtInst->GetTeamID()) return;

    if (UWorld* world = GetWorld()) world->GetTimerManager().ClearTimer(TH_ForgetTarget);

    blackboard.TargetActor = Seen;
    blackboard.LastKnownLocation = Seen->GetActorLocation();

    if (activeSequence) activeSequence->HandleSensedSight(Seen);
}

void UEnemyBrainComponent::HandleLostSight(AActor* Lost)
{
    if (!bActive || !Lost || blackboard.TargetActor != Lost || blackboard.bForgotTarget || !EnsureReferences()) return;

    blackboard.LastKnownLocation = Lost->GetActorLocation();
    if (UWorld* world = GetWorld()) world->GetTimerManager().SetTimer(TH_ForgetTarget, this, &UEnemyBrainComponent::HandleForgetSeenTarget, forgetSeenActorGracePeriod, false);

    if (activeSequence) activeSequence->HandleLostSight(Lost);
}

void UEnemyBrainComponent::HandleForgetSeenTarget()
{
    if (!bActive || !blackboard.TargetActor || !EnsureReferences()) return;
    if (activeSequence) activeSequence->HandleForgetSeenTarget(blackboard.TargetActor);
}

void UEnemyBrainComponent::HandleEQSQueryFinished(const FEnvQueryResult& Result)
{
    if (!bActive || !EnsureReferences()) return;

    blackboard.EQS_Actors.Empty();
    blackboard.EQS_Locs.Empty();

    Result.GetAllAsActors(blackboard.EQS_Actors);
    Result.GetAllAsLocations(blackboard.EQS_Locs);

    if (activeSequence) activeSequence->HandleEQSFinished(Result);
}

void UEnemyBrainComponent::HandleMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
    if (!bActive || !EnsureReferences()) return;
    if (activeSequence) activeSequence->HandleMoveCompleted(RequestID.GetID(), Result);
}

void UEnemyBrainComponent::HandleAnimNotify(const FGameplayTag& NotifyTag)
{
    if (!bActive || !EnsureReferences()) return;
    if (activeSequence) activeSequence->HandleAnimNotify(NotifyTag);
}

void UEnemyBrainComponent::HandleReceiveHitPre(FAtkHitData& HitData)
{
    if (!bActive || bEvaluatingReactive || blackboard.bForgotTarget || !EnsureReferences() || (activeSequence && !activeSequence->IsInterruptibleReactive())) return;

    bEvaluatingReactive = true;

    if (UEnemSeqReactive* potentialSequence = GetBestSequenceReactive(HitData))
    {
        if (activeSequence) DeactivateSequence();
        bEvaluatingReactive = false;
        ActivateSequenceReactive(potentialSequence, HitData);
    }
    else bEvaluatingReactive = false;
}

void UEnemyBrainComponent::HandleReceiveHitPost(const FAtkHitData& HitData)
{
    if (!bActive || blackboard.bForgotTarget || !EnsureReferences()) return;

    blackboard.LastDamageSource = HitData.attacker;
    if (CountsAsHit(HitData)) ++blackboard.RecentHits;
    if (UWorld* world = GetWorld())
    {
        lastHitTime = world->GetTimeSeconds();
        SetComponentTickEnabled(true); // Enable ticking so that the recent hits can be reset after a delay
    }

    if (activeSequence) activeSequence->HandleReceiveHitPost(HitData);
}

void UEnemyBrainComponent::HandleCountered(AActor* Counteror, const FString& Reason)
{
    if (!bActive || blackboard.bForgotTarget || !EnsureReferences()) return;

    blackboard.LastDamageSource = Counteror;
    blackboard.RecentHits = 0; // Reset recent hits on counter
}

bool UEnemyBrainComponent::CountsAsHit(const FAtkHitData& HitData) const
{
    return HitData.resolvedReaction != EHitReaction::None && HitData.resolvedReaction != EHitReaction::BlockHit
    && HitData.resolvedReaction != EHitReaction::BlockPerfect && HitData.resolvedReaction != EHitReaction::BlockBreak;
}