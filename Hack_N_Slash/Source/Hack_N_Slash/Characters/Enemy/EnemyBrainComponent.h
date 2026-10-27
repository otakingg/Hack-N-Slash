#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "EnemyBrainComponent.generated.h"

// This class handles the Enemy AI logic
// The brain is event based, meaning enemies will only react to events and not constantly tick

// It stores UObjects called "Enemy Sequences"
// The sequences represnt the actions an enemy can take
// All sequences inherit from the "EnemySequence" class, but can have their own custom blueprint code
// Sequence are spliut into "Proactive" & "Reactive" sequences
// Proactive: Attacking, Strafing, Flanking, etc.
// Reactive: Blocking, Dodging, etc.

class AEnemyController;
class ICombatInstigator;
class UEnemySequence;
class UEnemSeqProactive;
class UEnemSeqReactive;
class UStateMachineComponent;
struct FAtkHitData;
struct FEnvQueryResult;
struct FGameplayTag;

USTRUCT(BlueprintType)
struct FEnemyBlackboard // Custom blackboard
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite) AActor* TargetActor = nullptr;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)  double TargetDistance = -1.0f;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)  double TargetHeightDifference = 0.0f;
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite) FVector LastKnownLocation = FVector::ZeroVector;
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite) AActor* LastDamageSource = nullptr;
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite) FVector HomeLocation = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite) float Aggro = 0.0f;
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite) float LastAttackTime = -1.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)  TArray<AActor*> EQS_Actors;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)  TArray<FVector> EQS_Locs;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)  bool bLockedOn = false;
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite) bool bForgotTarget = false;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HACK_N_SLASH_API UEnemyBrainComponent : public UActorComponent
{
    GENERATED_BODY()

private:
    UPROPERTY(Transient) APawn* owner = nullptr;
    UPROPERTY(Transient) AEnemyController* controller = nullptr;
    UPROPERTY(Transient) UStateMachineComponent* smComp = nullptr;
    ICombatInstigator* iCmbtInst = nullptr;

    FTimerHandle TH_Decision; // Timer handle for proactive decision making
    FTimerHandle TH_ForgetTarget; // Timer handle for forgetting this enemy's target

    float forgetSeenActorGracePeriod = 5.0f;

    bool bReevaluationRequested = false; // Was an evaluation requested? Should the enemy think?
    bool bEvaluatingProactive = false; // Currnetly evaluating proactive sequences?
    bool bEvaluatingReactive = false; // Currently evaluating reactive sequences?

    bool EnsureReferences();
    void InitializeSequences();

    void DecisionTick(); // Runs every "decisionInterval" seconds
    void CalculateTargetDistance();
    void EvaluateSequencesProactive();
    UEnemSeqProactive* GetSequenceOffCoolDownProactive() const;
    UEnemSeqProactive* GetBestScoredSequenceProactive() const;
    UEnemSeqReactive* GetBestScoredSequenceReactive(const FAtkHitData& HitData) const;

    /** Event handlers */
    void HandleSensedSight(AActor* SeenActor);
    void HandleLostSight(AActor* LostActor);
    void HandleForgetSeenTarget();
    //void HandleSensedSound(AActor* HeardActor, const FVector& SoundOrigin);
    void HandleEQSQueryFinished(const FEnvQueryResult& Result);
    void HandleMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result);

protected:
    UPROPERTY(EditAnywhere, Category = "Brain")
    bool bDebug = false;

    UPROPERTY(VisibleAnywhere, Category = "Brain")
    bool bActive = true;

    UPROPERTY(EditDefaultsOnly, Category = "Brain")
    float decisionInterval = 0.2f;



    UPROPERTY(EditAnywhere, Category = "Brain|Aggro", meta = (ClampMin = "0.01", ClampMax = "1.0"))
    float aggroDecayRate = 0.08f;

    UPROPERTY(EditAnywhere, Category = "Brain|Aggro")
    float aggroDecayDelay = 3.0f;

    UPROPERTY(VisibleAnywhere, Category = "Brain|Aggro")
    float lastAggroTime = 0.0f;



    UPROPERTY(EditDefaultsOnly, Category = "Brain|Sequences", meta = (ClampMin = "0", ClampMax = "1", ToolTip = "Low = Allow lower scores, High = Require higher scores. Proactive sequences only"))
    float selectionThreshold = 0.7f; // Sequences need to score within this percent of the highest scoring sequence to be selectable

    UPROPERTY(EditDefaultsOnly, Category = "Brain|Sequences", meta = (ClampMin = "0.1", ToolTip = "How long after trying a reaction before the AI can try again. Maximum"))
    int reactionCooldownMax = 10;

    UPROPERTY(EditDefaultsOnly, Category = "Brain|Sequences", meta = (ClampMin = "0.1", ToolTip = "How long after trying a reaction before the AI can try again. Minimum"))
    int reactionCooldownMin = 0;

    UPROPERTY(VisibleAnywhere, Category = "Brain|Sequences", meta = (ToolTip = "The current cooldown for reactions that was randomly chosen between the min and max values"))
    int currentReactionCooldown = -1;

    UPROPERTY(VisibleAnywhere, Category = "Brain|Sequences")
    float lastReactionEvalTime = -1.0f;

    UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Brain|Sequences")
    UEnemySequence* activeSequence = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Brain|Sequences")
    TArray<TSubclassOf<UEnemSeqProactive>> proactiveSequenceClasses; // Stores the classes of the proactive enemy sequences to instantiate

    UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Brain|Sequences")
    TArray<UEnemSeqProactive*> proactiveSequenceInstances; // Stores the instantiated proactive enemy sequences

    UPROPERTY(EditDefaultsOnly, Category = "Brain|Sequences")
    TArray<TSubclassOf<UEnemSeqReactive>> reactiveSequenceClasses; // Stores the classes of the reactive enemy sequences to instantiate

    UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Brain|Sequences")
    TArray<UEnemSeqReactive*> reactiveSequenceInstances; // Stores the instantiated reactive enemy sequences

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UFUNCTION(BlueprintCallable, Category = "Brain")
    void RequestEvaluate();


public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brain")
    FEnemyBlackboard blackboard;

    UPROPERTY(VisibleAnywhere, Category = "Brain|Sequences")
    FName prevSequenceName = NAME_None;

    UEnemyBrainComponent();

    UFUNCTION(BlueprintPure, Category = "Brain")
    AEnemyController* GetEnemyController() const { return controller; }

    UFUNCTION(BlueprintPure, Category = "Brain")
    UStateMachineComponent* GetStateMachine() const { return smComp; }

    UFUNCTION(BlueprintCallable, Category = "Brain")
    void PauseBrain();

    UFUNCTION(BlueprintCallable, Category = "Brain")
    void UnpauseBrain();

    UFUNCTION(BlueprintCallable, Category = "Brain")
    void ResetBrain();

    UFUNCTION(BlueprintPure, Category = "Brain")
    UEnemySequence* GetActiveSequence() const { return activeSequence; }

    void ActivateSequence(UEnemySequence* Sequence);
    void DeactivateSequence();

    UFUNCTION(BlueprintCallable, Category = "Brain")
    void RemoveActiveSequence(bool bRequestRevaluation); // Used by enemy sequences to null themselves out

    /** Event handlers */
    void HandleAnimNotify(const FGameplayTag& NotifyTag);
    void HandleReceiveHitPre(FAtkHitData& HitData); // Logic before calculating damage. For custom logic like: Blocking, dodging, nullifying specific attaks, etc.
    void HandleReceiveHitPost(const FAtkHitData& HitData); // Logic after calculating damage. For cusotm logic like: Phase shifts
    void HandleCountered(AActor* Counteror, const FString& Reason);
};