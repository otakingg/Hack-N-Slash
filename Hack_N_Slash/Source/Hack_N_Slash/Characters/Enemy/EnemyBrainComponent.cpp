#include "EnemyBrainComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../../Tags/AnimNotifyTags.h"
#include "../../Tags/CharacterStateTagNamespaces.h"
#include "../../Controllers/EnemyController.h"
#include "Sequences/EnemySequence.h"
#include "../../Structs/FAtkHitData.h"
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

    if (!EnsureReferences()) return;
    InitializeSequences();

    if (controller)
    {
        forgetSeenActorGracePeriod = controller->GetMaxAgeSight();

        controller->OnSensedSight.AddUObject(this, &UEnemyBrainComponent::HandleSensedSight);
        controller->OnLostSight.AddUObject(this, &UEnemyBrainComponent::HandleLostSight);
        controller->OnSensedSound.AddUObject(this, &UEnemyBrainComponent::HandleSensedSound);
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

        blackboard.Aggro = FMath::Clamp(blackboard.Aggro, 0.0f, 1.0f);
    }

    // Example AI behavior checks
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

void UEnemyBrainComponent::ActivateBrain()
{
    UWorld* world = GetWorld();
    if (!world) return;

    if (controller) controller->Possess(ownerChar);
    SetComponentTickEnabled(true);

    bActive = true;
    FTimerManager& timerManager = world->GetTimerManager();
    timerManager.UnPauseTimer(TH_Decision);
    timerManager.UnPauseTimer(TH_ForgetTarget);
}

void UEnemyBrainComponent::DeactivateBrain()
{
    UWorld* world = GetWorld();
    if (!world) return;

    DeactivateSequence();
    SetComponentTickEnabled(false);

    FTimerManager& timerManager = world->GetTimerManager();
    timerManager.PauseTimer(TH_Decision);
    timerManager.PauseTimer(TH_ForgetTarget);
    bActive = false;

    EnsureReferences();
    if (moveComp) moveComp->StopMovementImmediately();
    if (locoComp) locoComp->ClearRootMotionSource();
    if (controller) controller->UnPossess();
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

    if (!stateMachineComp) stateMachineComp = ownerChar->FindComponentByClass<UStateMachineComponent>();
    if (!stateMachineComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UEnemyBrainComponent] No StateMachineComponent on: %s"), *GetNameSafe(ownerChar));
        return false;
    }

    if (!locoComp) locoComp = ownerChar->FindComponentByClass<ULocomotionComponent>();
    if (!locoComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UEnemyBrainComponent] No LocomotionComponent on: %s"), *GetNameSafe(ownerChar));
        return false;
    }

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
        return;
    }
    blackboard.TargetDistance = FVector::Dist(ownerChar->GetActorLocation(), blackboard.TargetActor->GetActorLocation());
}

void UEnemyBrainComponent::RequestReevaluate() { bReevaluationRequested = true; }

/*void UEnemyBrainComponent::EvaluateSequences()
{
    if (bEvaluating || (activeSequence && !activeSequence->bInterruptible)) return;
    if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("[EnemyBrainComp] Evaluating"));

    bEvaluating = true;

    float bestScore = -1.0f;
    UEnemySequence* bestSequence = nullptr;

    for (UEnemySequence* sequence : sequenceInstances)
    {
        if (!sequence || !sequence->CanExecute()) continue;

        float score = sequence->GetScore();
        if (score > bestScore)
        {
            bestScore = score;
            bestSequence = sequence;
        }
    }

    if (bestSequence != activeSequence)
    {
        if (activeSequence)
        {
            if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("[EnemyBrainComp] Interrupting Sequence"));
            DeactivateSequence();
        }
        ActivateSequence(bestSequence);
    }
    
    bEvaluating = false;
}*/

void UEnemyBrainComponent::EvaluateSequences()
{
    if (bEvaluating || (activeSequence && !activeSequence->bInterruptible)) return;
    if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("[EnemyBrainComp] Evaluating"));

    bEvaluating = true;

    TArray<UEnemySequence*> validSequences;
    TArray<float> scores;

    UEnemySequence* bestSequence = nullptr;
    float bestScore = -1.f;

    // -------------------------
    // 1. Gather scores
    // -------------------------
    for (UEnemySequence* sequence : sequenceInstances)
    {
        if (!sequence || !sequence->CanExecute()) continue;

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

    if (validSequences.Num() == 0)
    {
        bEvaluating = false;
        return;
    }

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

    for (int32 i = 0; i < 100; ++i)
    {
        int index = FMath::RandRange(0, size - 1);
        if (scores[index] > 0.0f)
        {
            bestSequence = validSequences[index];
            break;
        }
    }

    if (bestSequence != activeSequence)
    {
        if (activeSequence)
        {
            if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("[EnemyBrainComp] Interrupting Sequence"));
            DeactivateSequence();
        }
        ActivateSequence(bestSequence);
    }
    
    bEvaluating = false;
}

/*void UEnemyBrainComponent::EvaluateSequences()
{
    if (bEvaluating || (activeSequence && !activeSequence->bInterruptible)) return;

    if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("[EnemyBrainComp] Evaluating"));

    bEvaluating = true;

    TArray<UEnemySequence*> ValidSequences;
    TArray<float> Scores;

    float bestScore = -1.f;

    // -------------------------
    // 1. Gather scores
    // -------------------------
    for (UEnemySequence* sequence : sequenceInstances)
    {
        if (!sequence || !sequence->CanExecute()) continue;

        float score = sequence->GetScore();

        ValidSequences.Add(sequence);
        Scores.Add(score);

        bestScore = FMath::Max(bestScore, score);
    }

    if (ValidSequences.Num() == 0)
    {
        bEvaluating = false;
        return;
    }

    // -------------------------
    // 2. Apply selection bias + clamp weak options
    // -------------------------
    float totalWeight = 0.f;

    for (int32 i = 0; i < Scores.Num(); i++)
    {
        // Normalize relative to best (important!)
        float normalized = (bestScore > 0.f) ? (Scores[i] / bestScore) : 0.f;

        // Remove weak moves (optional but recommended)
        if (normalized < selectionFloor)
        {
            Scores[i] = 0.f;
            continue;
        }

        // Bias curve (THIS is your “controlled randomness”)
        float weighted = FMath::Pow(normalized, selectionBias);

        Scores[i] = weighted;
        totalWeight += weighted;
    }

    // -------------------------
    // 3. Pick weighted random
    // -------------------------
    float roll = FMath::FRandRange(0.f, totalWeight);

    float running = 0.f;
    UEnemySequence* chosen = nullptr;

    for (int32 i = 0; i < ValidSequences.Num(); i++)
    {
        running += Scores[i];

        if (roll <= running)
        {
            chosen = ValidSequences[i];
            break;
        }
    }

    // Fallback safety
    if (!chosen) chosen = ValidSequences[0];

    // -------------------------
    // 4. Switch sequences
    // -------------------------
    if (chosen != activeSequence)
    {
        if (activeSequence)
        {
            if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("[EnemyBrainComp] Interrupting Sequence"));
            DeactivateSequence();
        }
        ActivateSequence(chosen);
    }

    bEvaluating = false;
}*/

void UEnemyBrainComponent::ActivateSequence(UEnemySequence* Sequence)
{
    if (!Sequence) return;

    activeSequence = Sequence;
    activeSequence->Execute();
}

void UEnemyBrainComponent::DeactivateSequence()
{
    if (!activeSequence) return;
    activeSequence->Finish();
}

void UEnemyBrainComponent::RemoveActiveSequence()
{
    if (activeSequence) activeSequence = nullptr;
    RequestReevaluate();
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
    RequestReevaluate();
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
    RequestReevaluate();
}

void UEnemyBrainComponent::HandleForgetSeenTarget()
{
    if (!bActive || !EnsureReferences() || !blackboard.TargetActor || !controller) return;

    if (activeSequence) activeSequence->HandleForgetSeenTarget(blackboard.TargetActor);
    RequestReevaluate();
}

void UEnemyBrainComponent::HandleSensedSound(AActor* Heard, const FVector& Origin)
{
    if (!bActive || !EnsureReferences() || blackboard.bForgotTarget) return;
    if (!blackboard.TargetActor) blackboard.LastKnownLocation = Origin;
    if (activeSequence) activeSequence->HandleSensedSound(Heard, Origin);
    RequestReevaluate();
}

void UEnemyBrainComponent::HandleEQSQueryFinished(const FEnvQueryResult& Result)
{
    if (!bActive || !EnsureReferences()) return;

    blackboard.EQS_Actors.Empty();
    blackboard.EQS_Locs.Empty();

    Result.GetAllAsActors(blackboard.EQS_Actors);
    Result.GetAllAsLocations(blackboard.EQS_Locs);

    if (activeSequence) activeSequence->HandleEQSFinished(Result);
    RequestReevaluate();
}

void UEnemyBrainComponent::HandleMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
    if (!bActive || !EnsureReferences()) return;
    if (activeSequence) activeSequence->HandleMoveCompleted(RequestID.GetID(), Result);
    RequestReevaluate();
}

void UEnemyBrainComponent::HandleAnimNotify(const FGameplayTag& NotifyTag)
{
    if (!bActive || !EnsureReferences()) return;

    if (NotifyTag.MatchesTagExact(EnemyBrainTags::ClearStagger)) blackboard.bStaggered = false;
    else if (activeSequence) activeSequence->HandleAnimNotify(NotifyTag);
    RequestReevaluate();
}

void UEnemyBrainComponent::HandleAttackDetected()
{
    if (!bActive || !EnsureReferences() || blackboard.bForgotTarget) return;
    if (activeSequence) activeSequence->HandleAttackDetected();
    RequestReevaluate();
}

void UEnemyBrainComponent::HandleReceiveHitPre(FAtkHitData& HitData)
{
    if (!bActive || !EnsureReferences() || blackboard.bForgotTarget) return;
    if (activeSequence) activeSequence->HandleReceiveHitPre(HitData);
    RequestReevaluate();
}

void UEnemyBrainComponent::HandleReceiveHitPost(FAtkHitData& HitData)
{
    if (!bActive || !EnsureReferences() || blackboard.bForgotTarget) return;
    blackboard.LastDamageSource = HitData.attacker;
    
    if (HitData.resolvedReaction != ActionTags::None)
    {
        blackboard.bStaggered = true;
        DeactivateSequence();
        /*if (locoComp) locoComp->ClearRootMotionSource();
        if (moveComp) moveComp->StopMovementImmediately();
        controller->StopMovement();*/
    }

    if (HitData.dmgHPDealt > 0.0f)
    {
        blackboard.Aggro += HitData.aggroBuildup;
        blackboard.Aggro = FMath::Clamp(blackboard.Aggro, 0.0, 1.0f);
        if (!IsComponentTickEnabled()) SetComponentTickEnabled(true);
    }

    if (activeSequence) activeSequence->HandleReceiveHitPost(HitData);
    RequestReevaluate();
}

void UEnemyBrainComponent::HandleCountered(AActor* Counteror, const FString& Reason)
{
    if (!bActive || !EnsureReferences() || blackboard.bForgotTarget) return;

    blackboard.LastDamageSource = Counteror;
    blackboard.bStaggered = true;
    blackboard.Aggro += 0.1f;
    blackboard.Aggro = FMath::Clamp(blackboard.Aggro, 0.0, 1.0f);
    if (!IsComponentTickEnabled()) SetComponentTickEnabled(true);
    DeactivateSequence();

    if (activeSequence) activeSequence->OnCountered(Counteror, Reason);
    RequestReevaluate();
}