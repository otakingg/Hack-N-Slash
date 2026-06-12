#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Navigation/PathFollowingComponent.h"
#include "EnemyBrainComponent.generated.h"

class AEnemyController;
class UCapsuleComponent;
class UCharacterMovementComponent;
class UEnemySequence;
class ULocomotionComponent;
class UStateMachineComponent;
struct FAtkHitData;
struct FEnvQueryResult;

USTRUCT(BlueprintType)
struct FEnemyBlackboard
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
    UPROPERTY(Visibleanywhere, BlueprintReadOnly) int32 ConsecutiveSequenceUses = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TArray<AActor*> EQS_Actors;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TArray<FVector> EQS_Locs;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite) bool bForgotTarget = false;
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite) bool bStaggered = false;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HACK_N_SLASH_API UEnemyBrainComponent : public UActorComponent
{
    GENERATED_BODY()

private:
    UPROPERTY(Transient) ACharacter* ownerChar = nullptr;
    UPROPERTY(Transient) USkeletalMeshComponent* meshComp = nullptr;
    UPROPERTY(Transient) UCharacterMovementComponent* moveComp = nullptr;
    UPROPERTY(Transient) UCapsuleComponent* capsuleComp = nullptr;
    UPROPERTY(Transient) AEnemyController* controller = nullptr;
    UPROPERTY(Transient) UStateMachineComponent* stateMachineComp = nullptr;
    UPROPERTY(Transient) ULocomotionComponent* locoComp = nullptr;

    FTimerHandle TH_Wait;
    FTimerHandle TH_Decision;
    FTimerHandle TH_ForgetTarget;

    float forgetSeenActorGracePeriod = 5.0f;

    bool bReevaluationRequested = false;
    bool bEvaluating = false;

    bool EnsureReferences();
    void InitializeSequences();

    void DecisionTick();
    void CalculateTargetDistance();
    void EvaluateSequences();

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

    UPROPERTY(EditDefaultsOnly, Category = "Brain|Sequences", meta = (ClampMin = "0.1", ClampMax = "1.0", ToolTip = "Low = Allow lower scores, High = Require higher scores"))
    float selectionFloor = 0.5f;

    UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Brain|Sequences")
    UEnemySequence* activeSequence = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Brain|Sequences")
    TArray<TSubclassOf<UEnemySequence>> sequenceClasses;

    UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Brain|Sequences")
    TArray<UEnemySequence*> sequenceInstances;

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brain")
    FEnemyBlackboard blackboard;

    UPROPERTY(VisibleAnywhere, Category = "Brain")
    FName prevSequenceName = NAME_None;

    UEnemyBrainComponent();

    UFUNCTION(BlueprintPure, Category = "Brain")
    UCapsuleComponent* GetCapsuleComponent() const { return capsuleComp; }

    UFUNCTION(BlueprintPure, Category = "Brain")
    UCharacterMovementComponent* GetCharacterMovement() const { return moveComp; }

    UFUNCTION(BlueprintPure, Category = "Brain")
    AEnemyController* GetEnemyController() const { return controller; }

    UFUNCTION(BlueprintPure, Category = "Brain")
    TArray<UEnemySequence*> GetEnemySequences() const { return sequenceInstances; }

    UFUNCTION(BlueprintPure, Category = "Brain")
    ULocomotionComponent* GetLocoMotionComp() const { return locoComp; }

    UFUNCTION(BlueprintPure, Category = "Brain")
    USkeletalMeshComponent* GetMeshComp() const { return meshComp; }

    UFUNCTION(BlueprintPure, Category = "Brain")
    UStateMachineComponent* GetStateMachine() const { return stateMachineComp; }

    UFUNCTION(BlueprintCallable, Category = "Brain")
    void PauseBrain();

    UFUNCTION(BlueprintCallable, Category = "Brain")
    void UnpauseBrain();

    UFUNCTION(BlueprintCallable, Category = "Brain")
    void ResetBrain();

    UFUNCTION(BlueprintPure, Category = "Brain")
    UEnemySequence* GetEnemySequence(FName SequenceName) const;
    void ActivateSequence(UEnemySequence* Sequence);
    void DeactivateSequence();
    UFUNCTION(BlueprintCallable, Category = "Brain")
    void RemoveActiveSequence(); // Used by enemy sequences to null out the active sequence. Mainly used when a sequence finishes so it can be picked again if it's still the best choice

    UFUNCTION(BlueprintCallable, Category = "Brain")
    void RequestReevaluate();

    void HandleAnimNotify(const FGameplayTag& NotifyTag);
    void HandleAttackDetected(); // Geing targetted for an attack, but the attack hasn't hit yet
    void HandleReceiveHitPre(FAtkHitData& HitData); // Logic before calculating damage. For custom logic like: Blocking, dodging, nullifying specific attaks, etc. 
    void HandleReceiveHitPost(FAtkHitData& HitData); // Logic after calculating damage. For cusotm logic like: Phase shifts, retaliation, updating atk preferences, etc.
    void HandleCountered(AActor* Counteror, const FString& Reason);
};