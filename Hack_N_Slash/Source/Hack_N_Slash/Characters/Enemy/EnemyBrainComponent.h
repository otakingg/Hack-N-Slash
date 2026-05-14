#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Navigation/PathFollowingComponent.h"
#include "EnemyBrainComponent.generated.h"

class AEnemyController;
class UEnemyBrainModule;
class UEnemySequence;
class UStateMachineComponent;
struct FAtkHitData;
struct FEnvQueryResult;

USTRUCT(BlueprintType)
struct FEnemyBlackboard
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite) AActor* TargetActor = nullptr;
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite) FVector LastKnownLocation = FVector::ZeroVector;
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite) AActor* LastDamageSource = nullptr;
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite) FVector HomeLocation = FVector::ZeroVector;

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
    UPROPERTY() AEnemyController* controller = nullptr;
    UPROPERTY() UStateMachineComponent* stateMachineComp = nullptr;

    FTimerHandle TH_Wait;
    FTimerHandle TH_Decision;
    FTimerHandle TH_ForgetTarget;

    float forgetSeenActorGracePeriod = 5.0f;

    bool bReevaluationRequested = false;
    bool bEvaluating = false;

    void CachePointers();
    void InitializeModulesAndSequences();

    void DecisionTick();
    void EvaluateModules(const FString& Reason);

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

    UPROPERTY(EditAnywhere, Category = "Brain")
    bool bActive = true;

    UPROPERTY(EditDefaultsOnly, Category = "Brain")
    float decisionInterval = 0.2f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category = "Brain|Modules")
    UEnemyBrainModule* activeModule = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Brain|Modules")
    TArray<TSubclassOf<UEnemyBrainModule>> moduleClasses;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category = "Brain|Modules")
    TArray<UEnemyBrainModule*> moduleInstances;

    UPROPERTY(EditDefaultsOnly, Category = "Brain|Enemy Sequences")
    TArray<TSubclassOf<UEnemySequence>> sequenceClasses;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category = "Brain|Enemy Sequences")
    TArray<UEnemySequence*> sequenceInstances;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brain")
    FEnemyBlackboard blackboard;

    UEnemyBrainComponent();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UFUNCTION(BlueprintPure)
    AEnemyController* GetEnemyController() const { return controller; }

    UFUNCTION(BlueprintPure)
    TArray<UEnemySequence*> GetEnemySequences() const { return sequenceInstances; }

    UFUNCTION(BlueprintPure)
    UStateMachineComponent* GetStateMachine() const { return stateMachineComp; }

    void ActivateBrain();
    void DeactivateBrain();

    void ActivateModule(UEnemyBrainModule* Module);
    UFUNCTION(BlueprintCallable, Category = "Brain")
    void DeactivateModule();

    UFUNCTION(BlueprintCallable)
    void RequestReevaluate();

    void HandleAnimNotify(FGameplayTag NotifyTag);
    UFUNCTION() void HandleMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);

    void HandleReceiveHitPre(FAtkHitData& HitData); // Logic before calculating damage. For custom logic like: Blocking, dodging, nullifying specific attaks, etc. 
    void HandleReceiveHitPost(FAtkHitData& HitData); // Logic after calculating damage. For cusotm logic like: Phase shifts, retaliation, updating atk preferences, etc.
    void HandleAttackDetected(); // Geing targetted for an attack, but the attack hasn't hit yet
};