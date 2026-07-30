#include "EnemyBrainComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../../Animation/AnimInstances/BaseCharAnimInstance.h"
#include "../../Combat/Shared/CombatResolutionComponent.h"
#include "../../Combat/Enemy/EnemyCombatComponent.h"
#include "../../Controllers/EnemyController.h"
#include "Sequences/EnemySequence.h"
#include "../../Structs/FAtkHitData.h"
#include "../Shared/LocomotionComponent.h"
#include "../Shared/StateMachineComponent.h"
#include "../Shared/StatsComponent.h"

UEnemyBrainComponent::UEnemyBrainComponent() { PrimaryComponentTick.bCanEverTick = true; }

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
    if (!world || !EnsureReferences()) return;

    InitializeSequences();

    blackboard.HomeLocation = ownerChar->GetActorLocation();

    if (controller)
    {
        forgetSeenActorGracePeriod = controller->GetMaxAgeSight();

        controller->OnSensedSight.AddUObject(this, &UEnemyBrainComponent::HandleSensedSight);
        controller->OnLostSight.AddUObject(this, &UEnemyBrainComponent::HandleLostSight);
        controller->OnSensedSound.AddUObject(this, &UEnemyBrainComponent::HandleSensedSound);
        controller->OnEQSQueryFinishedDel.AddUObject(this, &UEnemyBrainComponent::HandleEQSQueryFinished);
        controller->OnMoveCompletedDel.AddUObject(this, &UEnemyBrainComponent::HandleMoveCompleted);
    }

    if (bActive)
    {
        RequestEvaluate();
        world->GetTimerManager().SetTimer(TH_Decision, this, &UEnemyBrainComponent::DecisionTick, decisionInterval, true);
    }
}

void UEnemyBrainComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UWorld* world = GetWorld();
    if (!world) return;

    const float timeSinceLastAggro = world->GetTimeSeconds() - lastAggroTime;

    // Only decay after delay
    if (timeSinceLastAggro >= aggroDecayDelay)
    {
        blackboard.Aggro -= activeAggroDecayRate * DeltaTime;
        blackboard.Aggro = FMath::Clamp(blackboard.Aggro, 0.0f, 1.0f);
    }

    // For performance
    if (blackboard.Aggro <= 0.0f) SetComponentTickEnabled(false);

}

void UEnemyBrainComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    DeactivateSequence();

    sequenceInstances.Empty();

    if (controller)
    {
        controller->OnSensedSight.RemoveAll(this);
        controller->OnLostSight.RemoveAll(this);
        controller->OnSensedSound.RemoveAll(this);
        controller->OnEQSQueryFinishedDel.RemoveAll(this);
        controller->OnMoveCompletedDel.RemoveAll(this);
    }

    if (UWorld* world = GetWorld()) world->GetTimerManager().ClearAllTimersForObject(this);
    
    Super::EndPlay(EndPlayReason);
}

void UEnemyBrainComponent::PauseBrain()
{
    bActive = false;
    UWorld* world = GetWorld();
    if (!world) return;
    
    if (blackboard.Aggro > 0.0f) SetComponentTickEnabled(false);

    FTimerManager& timerManager = world->GetTimerManager();
    timerManager.PauseTimer(TH_Decision);
    if (timerManager.IsTimerActive(TH_ForgetTarget)) timerManager.PauseTimer(TH_ForgetTarget);
}

void UEnemyBrainComponent::UnpauseBrain()
{
    UWorld* world = GetWorld();
    if (!world) return;

    if (controller) controller->Possess(ownerChar);
    if (blackboard.Aggro > 0.0f) SetComponentTickEnabled(true);

    bActive = true;
    FTimerManager& timerManager = world->GetTimerManager();
    timerManager.UnPauseTimer(TH_Decision);
    if (timerManager.IsTimerPaused(TH_ForgetTarget)) timerManager.UnPauseTimer(TH_ForgetTarget);
    RequestEvaluate();
}

void UEnemyBrainComponent::ResetBrain()
{
    if (UWorld* world = GetWorld()) world->GetTimerManager().ClearAllTimersForObject(this);

    while (true) if (!activeSequence) break;

    SetComponentTickEnabled(false);
    if (controller) controller->ClearFocusHNS();
    blackboard.Aggro = 0.0f;
    blackboard.bForgotTarget = false;
    blackboard.EQS_Actors.Empty();
    blackboard.EQS_Locs.Empty();
    blackboard.TargetActor = nullptr;
    blackboard.TargetDistance = -1.0f;
    blackboard.TargetHeightDifference = 0.0f;
    blackboard.LastAttackTime = -1.0f;
    blackboard.LastDamageSource = nullptr;
}

bool UEnemyBrainComponent::EnsureReferences()
{
    if (!ownerChar) ownerChar = Cast<ACharacter>(GetOwner());
    if (!ownerChar)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UEnemyBrainComponent] Owner is not an ACharacter: %s"), *GetNameSafe(GetOwner()));
        return false;
    }

    if (!meshComp) meshComp = ownerChar->GetComponentByClass<USkeletalMeshComponent>();
    if (!meshComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UEnemyBrainComponent] Owner skeletal mesh isn't valid: %s"), *GetNameSafe(GetOwner()));
        return false;
    }

    if (!animInstance) animInstance = Cast<UBaseCharAnimInstance>(meshComp->GetAnimInstance());
    if (!animInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UEnemyBrainComponent] Owner animation instance isn't valid: %s"), *GetNameSafe(GetOwner()));
        return false;
    }

    if (!controller) controller = Cast<AEnemyController>(ownerChar->GetController());
    if (!controller)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UEnemyBrainComponent] No EnemyController on: %s"), *GetNameSafe(ownerChar));
        return false;
    }

    if (!moveComp) moveComp = ownerChar->GetCharacterMovement();
    if (!moveComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UEnemyBrainComponent] No CharacterMovementComponent on: %s"), *GetNameSafe(ownerChar));
        return false;
    }

    if (!capsuleComp) capsuleComp = ownerChar->FindComponentByClass<UCapsuleComponent>();
    if (!capsuleComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UEnemyBrainComponent] No CapsuleComponent on: %s"), *GetNameSafe(ownerChar));
        return false;
    }

    if (!combatComp) combatComp = ownerChar->FindComponentByClass<UEnemyCombatComponent>();
    if (!combatResComp) combatResComp = ownerChar->FindComponentByClass<UCombatResolutionComponent>();
    if (!locoComp) locoComp = ownerChar->FindComponentByClass<ULocomotionComponent>();
    if (!stateMachineComp) stateMachineComp = ownerChar->FindComponentByClass<UStateMachineComponent>();
    if (!statsComp) statsComp = ownerChar->FindComponentByClass<UStatsComponent>();

    return true;
}

void UEnemyBrainComponent::InitializeSequences()
{
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
    if (!bActive || !EnsureReferences()) return;

    CalculateTargetDistance();
    
    if (!bReevaluationRequested) return;
    bReevaluationRequested = false;
    EvaluateSequences();
}

void UEnemyBrainComponent::CalculateTargetDistance()
{
    if (!blackboard.TargetActor)
    {
        blackboard.TargetDistance = -1.0f;
        blackboard.TargetHeightDifference = 0.0f;
        return;
    }

    FVector startLoc = ownerChar->GetActorLocation();
    FVector endLoc = blackboard.TargetActor->GetActorLocation();

    blackboard.TargetDistance = FVector::Dist(startLoc, endLoc);
    blackboard.TargetHeightDifference = (endLoc - startLoc).Z;
}

void UEnemyBrainComponent::RequestEvaluate() { bReevaluationRequested = true; }

void UEnemyBrainComponent::RequestSequence(FName SequenceName, bool bForce)
{
    if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("[EnemyBrainComp] Requesting Sequence"));

    UEnemySequence* potentialSequence = GetEnemySequence(SequenceName);
    if (!potentialSequence || !potentialSequence->CanExecute()) return;

    bool bCanExitActiveSequence = !activeSequence || activeSequence->bInterruptible || bForce;
    if (!bCanExitActiveSequence) return;

    bReevaluationRequested = false;

    if (activeSequence)
    {
        if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("[EnemyBrainComp] Interrupting Sequence"));
        DeactivateSequence();
    }
    ActivateSequence(potentialSequence);
}

void UEnemyBrainComponent::EvaluateSequences()
{
    if (bEvaluating || activeSequence) return;

    if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("[EnemyBrainComp] Evaluating"));

    bEvaluating = true;

    UEnemySequence* chosenSequence = nullptr;
    chosenSequence = PickSequenceOffCoolDown(); // If there's a sequence that wants to be played off cooldown ignoring scores, choose it
    if (!chosenSequence) chosenSequence = PickBestScoredSequence(); // Else pick the best scored sequence

    if (chosenSequence) ActivateSequence(chosenSequence);
    
    bEvaluating = false;
}

UEnemySequence* UEnemyBrainComponent::PickSequenceOffCoolDown()
{
    for (UEnemySequence* sequence : sequenceInstances) if (sequence && sequence->bInEvalCycle && sequence->bForceOffCooldown && sequence->CanExecute()) return sequence;
    return nullptr;
}

UEnemySequence* UEnemyBrainComponent::PickBestScoredSequence()
{
    TArray<UEnemySequence*> validSequences;
    TArray<float> scores;

    UEnemySequence* bestSequence = nullptr;
    float bestScore = -1.f;

    // -------------------------
    // 1. Gather scores
    // -------------------------
    for (UEnemySequence* sequence : sequenceInstances)
    {
        if (!sequence || !sequence->bInEvalCycle || !sequence->CanExecute()) continue;

        float score = sequence->GetScore();

        if (bDebug)
        {
            // Print to screen
            if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, FString::Printf(TEXT("Sequence: %s | Score: %.2f"), *sequence->GetName(), score));

            // Log to Output Log
            UE_LOG(LogTemp, Display, TEXT("Sequence: %s | Score: %.2f"), *sequence->GetName(), score);
        }

        validSequences.Add(sequence);
        scores.Add(score);

        bestScore = FMath::Max(bestScore, score);
    }

    if (validSequences.Num() == 0) return nullptr;

    // -------------------------
    // 2. Apply selection bias + clamp weak options
    // -------------------------

    int size = scores.Num();
    for (int32 i = 0; i < scores.Num(); i++)
    {
        // Normalize relative to best (important!)
        float normalized = (bestScore > 0.f) ? (scores[i] / bestScore) : 0.f;

        // Remove weak moves (optional but recommended)
        if (normalized < selectionFloor)
        {
            scores[i] = 0.f;
            continue;
        }
    }

    // -------------------------
    // 3. Pick a random sequence from the best ones
    // -------------------------

    // Try at most 100 times to get a random sequence with a good enough score. 100 times is a good number to check for performance
    // Remember that we'll only be checking for sequences within a "selectionFloor" % of the best sequence
    // All the sequences below that percent had their scores set to 0. So just keep checking until you find a sequence with a score > 0
    for (int32 i = 0; i < 100; ++i)
    {
        int index = FMath::RandRange(0, size - 1);
        if (scores[index] > 0.0f)
        {
            bestSequence = validSequences[index];
            break;
        }
    }
    return bestSequence;
}

UEnemySequence* UEnemyBrainComponent::GetEnemySequence(FName SequenceName) const
{
    for (UEnemySequence* sequence : sequenceInstances) if (sequence && sequence->GetSeqName() == SequenceName) return sequence;
    return nullptr;
}

void UEnemyBrainComponent::ActivateSequence(UEnemySequence* Sequence)
{
    if (!Sequence) return;

    if (prevSequenceName != Sequence->GetSeqName()) prevSequenceName = Sequence->GetSeqName();

    activeSequence = Sequence;
    activeSequence->Execute();
}

void UEnemyBrainComponent::DeactivateSequence()
{
    if (!activeSequence) return;

    UEnemySequence* oldSequence = activeSequence;
    activeSequence = nullptr;

    oldSequence->Abort();
}

void UEnemyBrainComponent::RemoveActiveSequence()
{
    if (activeSequence) activeSequence = nullptr;
    RequestEvaluate();
}

void UEnemyBrainComponent::HandleSensedSight(AActor* Seen)
{
    if (!bActive || !EnsureReferences() || !Seen || blackboard.bForgotTarget) return;

    UWorld* world = GetWorld();
    if (!world) return;

    world->GetTimerManager().ClearTimer(TH_ForgetTarget);

    activeAggroDecayRate = aggroDecayRateVisible;
    blackboard.TargetActor = Seen;
    blackboard.LastKnownLocation = Seen->GetActorLocation();

    if (activeSequence) activeSequence->HandleSensedSight(Seen);
    RequestEvaluate();
}

void UEnemyBrainComponent::HandleLostSight(AActor* Lost)
{
    if (!bActive || !EnsureReferences() || !blackboard.TargetActor || blackboard.bForgotTarget) return;

    if (blackboard.TargetActor == Lost)
    {
        activeAggroDecayRate = aggroDecayRateLostSight;
        blackboard.LastKnownLocation = Lost->GetActorLocation();
        if (UWorld* world = GetWorld()) world->GetTimerManager().SetTimer(TH_ForgetTarget, this, &UEnemyBrainComponent::HandleForgetSeenTarget, forgetSeenActorGracePeriod, false);
    }

    if (activeSequence) activeSequence->HandleLostSight(Lost);
    RequestEvaluate();
}

void UEnemyBrainComponent::HandleForgetSeenTarget()
{
    if (!bActive || !EnsureReferences() || !blackboard.TargetActor || !controller) return;

    if (activeSequence) activeSequence->HandleForgetSeenTarget(blackboard.TargetActor);
    RequestEvaluate();
}

void UEnemyBrainComponent::HandleSensedSound(AActor* Heard, const FVector& Origin)
{
    if (!bActive || !EnsureReferences() || blackboard.bForgotTarget) return;
    if (!blackboard.TargetActor) blackboard.LastKnownLocation = Origin;
    if (activeSequence) activeSequence->HandleSensedSound(Heard, Origin);
    RequestEvaluate();
}

void UEnemyBrainComponent::HandleEQSQueryFinished(const FEnvQueryResult& Result)
{
    if (!bActive || !EnsureReferences()) return;

    blackboard.EQS_Actors.Empty();
    blackboard.EQS_Locs.Empty();

    Result.GetAllAsActors(blackboard.EQS_Actors);
    Result.GetAllAsLocations(blackboard.EQS_Locs);

    if (activeSequence) activeSequence->HandleEQSFinished(Result);
    RequestEvaluate();
}

void UEnemyBrainComponent::HandleMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
    if (!bActive || !EnsureReferences()) return;
    if (activeSequence) activeSequence->HandleMoveCompleted(RequestID.GetID(), Result);
    RequestEvaluate();
}

void UEnemyBrainComponent::HandleAnimNotify(const FGameplayTag& NotifyTag)
{
    if (!bActive || !EnsureReferences()) return;

    if (NotifyTag.MatchesTagExact(Tags::NotifyEvent::EnemyBrain::RequestEvaluate)) RequestEvaluate();
    else if (activeSequence) activeSequence->HandleAnimNotify(NotifyTag);
}

void UEnemyBrainComponent::HandleAttackDetected(const FAtkData& AtkData)
{
    if (!bActive || !EnsureReferences() || blackboard.bForgotTarget) return;
    if (activeSequence) activeSequence->HandleAttackDetected(AtkData);
}

void UEnemyBrainComponent::HandleReceiveHitPre(FAtkHitData& HitData)
{
    if (!bActive || !EnsureReferences() || blackboard.bForgotTarget) return;
    if (activeSequence) activeSequence->HandleReceiveHitPre(HitData);
}

void UEnemyBrainComponent::HandleReceiveHitPost(FAtkHitData& HitData)
{
    if (!bActive || !EnsureReferences() || blackboard.bForgotTarget) return;

    blackboard.LastDamageSource = HitData.attacker;

    if (HitData.dmgHPDealt > 0.0f)
    {
        if (UWorld* world = GetWorld()) lastAggroTime = world->GetTimeSeconds();
        blackboard.Aggro += HitData.aggroBuildup;
        blackboard.Aggro = FMath::Clamp(blackboard.Aggro, 0.0, 1.0f);
        if (!IsComponentTickEnabled()) SetComponentTickEnabled(true);
    }

    if (activeSequence) activeSequence->HandleReceiveHitPost(HitData);
    RequestEvaluate();
}

void UEnemyBrainComponent::HandleCountered(AActor* Counteror, const FString& Reason)
{
    if (!bActive || !EnsureReferences() || blackboard.bForgotTarget) return;

    blackboard.LastDamageSource = Counteror;
    blackboard.Aggro += 0.1f;
    blackboard.Aggro = FMath::Clamp(blackboard.Aggro, 0.0, 1.0f);
    if (!IsComponentTickEnabled()) SetComponentTickEnabled(true);

    if (activeSequence) activeSequence->OnCountered(Counteror, Reason);
    RequestEvaluate();
}