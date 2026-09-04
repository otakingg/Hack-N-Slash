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
class UCapsuleComponent;
class UBaseCharAnimInstance;
class UCharacterMovementComponent;
class UCombatResolutionComponent;
class UEnemyCombatComponent;
class UEnemySequence;
class UEnemSeqProactive;
class UEnemSeqReactive;
class ULocomotionComponent;
class UStateMachineComponent;
class UStatsComponent;
struct FAtkHitData;
struct FEnvQueryResult;
struct FGameplayTag;

USTRUCT(BlueprintType)
struct FEnemyBlackboard // Custom blackboard
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite) AActor* TargetActor = nullptr;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) double TargetDistance = -1.0f;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) double TargetHeightDifference = 0.0f;
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite) FVector LastKnownLocation = FVector::ZeroVector;
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite) AActor* LastDamageSource = nullptr;
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite) FVector HomeLocation = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite) float Aggro = 0.0f;
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite) float LastAttackTime = -1.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TArray<AActor*> EQS_Actors;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TArray<FVector> EQS_Locs;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)  bool bLockedOn = false;
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite) bool bForgotTarget = false;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HACK_N_SLASH_API UEnemyBrainComponent : public UActorComponent
{
    GENERATED_BODY()

private:
    UPROPERTY(Transient) UBaseCharAnimInstance* animInstance = nullptr;
    UPROPERTY(Transient) ACharacter* ownerChar = nullptr;
    UPROPERTY(Transient) USkeletalMeshComponent* meshComp = nullptr;
    UPROPERTY(Transient) UCharacterMovementComponent* moveComp = nullptr;
    UPROPERTY(Transient) UCapsuleComponent* capsuleComp = nullptr;
    UPROPERTY(Transient) UEnemyCombatComponent* combatComp = nullptr;
    UPROPERTY(Transient) UCombatResolutionComponent* combatResComp = nullptr;
    UPROPERTY(Transient) AEnemyController* controller = nullptr;
    UPROPERTY(Transient) UStateMachineComponent* stateMachineComp = nullptr;
    UPROPERTY(Transient) UStatsComponent* statsComp = nullptr;
    UPROPERTY(Transient) ULocomotionComponent* locoComp = nullptr;

    FTimerHandle TH_Wait; // Timer handle used for waiting to initialize this brain. We wait to give time for the State Machine to initialize
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

    UFUNCTION() void Wait();

    /** Event handlers */
    void HandleSensedSight(AActor* SeenActor);
    void HandleLostSight(AActor* LostActor);
    void HandleForgetSeenTarget();
    void HandleSensedSound(AActor* HeardActor, const FVector& SoundOrigin);
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
    float aggroDecayRateVisible = 0.08f;

    UPROPERTY(EditAnywhere, Category = "Brain|Aggro", meta = (ClampMin = "0.01", ClampMax = "1.0"))
    float aggroDecayRateLostSight = 0.1f;

    UPROPERTY(VisibleAnywhere, Category = "Brain|Aggro", meta = (ClampMin = "0.01", ClampMax = "1.0"))
    float activeAggroDecayRate = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Brain|Aggro")
    float aggroDecayDelay = 3.0f;

    UPROPERTY(VisibleAnywhere, Category = "Brain|Aggro")
    float lastAggroTime = 0.0f;



    UPROPERTY(EditDefaultsOnly, Category = "Brain|Sequences", meta = (ClampMin = "0", ClampMax = "1", ToolTip = "Low = Allow lower scores, High = Require higher scores"))
    float selectionThreshold = 0.7f; // Sequences need to score within this percent of the highest scoring sequence to be selectable

    UPROPERTY(EditDefaultsOnly, Category = "Brain|Sequences", meta = (ClampMin = "0.1", ToolTip = "How long after evaluating a reaction before the AI can evaluate again"))
    float reactionEvalCooldown = 3.0f; // The enemy won't be able to evaluate reaction sequences for this long after the last time they were evaluated

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
    UBaseCharAnimInstance* GetAnimInstance() const { return animInstance; }

    UFUNCTION(BlueprintPure, Category = "Brain")
    UCapsuleComponent* GetCapsule() const { return capsuleComp; }

    UFUNCTION(BlueprintPure, Category = "Brain")
    UCharacterMovementComponent* GetCharacterMovement() const { return moveComp; }

    UFUNCTION(BlueprintPure, Category = "Brain")
    UCombatResolutionComponent* GetCombatResComp() const { return combatResComp; }

    UFUNCTION(BlueprintPure, Category = "Brain")
    UEnemyCombatComponent* GetEnemyCombatComp() const { return combatComp; }

    UFUNCTION(BlueprintPure, Category = "Brain")
    AEnemyController* GetEnemyController() const { return controller; }

    UFUNCTION(BlueprintPure, Category = "Brain")
    TArray<UEnemSeqProactive*> GetProactiveSequences() const { return proactiveSequenceInstances; }

    UFUNCTION(BlueprintPure, Category = "Brtain")
    TArray<UEnemSeqReactive*> GetReactiveSequences() const { return reactiveSequenceInstances; }

    UFUNCTION(BlueprintPure, Category = "Brain")
    ULocomotionComponent* GetLocoMotionComp() const { return locoComp; }

    UFUNCTION(BlueprintPure, Category = "Brain")
    USkeletalMeshComponent* GetMesh() const { return meshComp; }

    UFUNCTION(BlueprintPure, Category = "Brain")
    UStateMachineComponent* GetStateMachineComp() const { return stateMachineComp; }

    UFUNCTION(BlueprintPure, Category = "Brain")
    UStatsComponent* GetStatsComp() const { return statsComp; }

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