#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyBrainComponent.generated.h"

class AEnemyController;
class UEnemyBrainModule;
class UStateMachineComponent;
struct FAtkHitData;

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
    void InitializeModules();

    void DecisionTick();
    void EvaluateModules(const FString& Reason);

    void ActivateModule(UEnemyBrainModule* Module);
    void DeactivateModule(UEnemyBrainModule* Module);

    UFUNCTION() void Wait();

    /** Event handlers */
    UFUNCTION() void HandleSensedSight(AActor* SeenActor);
    UFUNCTION() void HandleLostSight(AActor* LostActor);
    UFUNCTION() void HandleForgetSeenTarget();
    UFUNCTION() void HandleSensedSound(AActor* HeardActor, const FVector& SoundOrigin);
    UFUNCTION() void HandleEQSQueryFinished(const FEnvQueryResult& Result);
    UFUNCTION() void HandleMoveCompleted(bool bSuccess);

protected:
    UPROPERTY(EditAnywhere, Category="Brain")
    bool bDebug = false;

    UPROPERTY(EditAnywhere, Category="Brain")
    bool bActive = true;

    /** Decision frequency
     * Decision rate = 5 Hz
     * Enemies = 30
     * Evaluations = 150/sec
     * Very cheap, nothing for UE5
    */
    UPROPERTY(EditDefaultsOnly, Category="Brain")
    float decisionInterval = 0.2f; // 5Hz

    /** Active module */
    UPROPERTY(VisibleAnywhere, Transient)
    UEnemyBrainModule* activeModule = nullptr;

    /** Module classes */
    UPROPERTY(EditDefaultsOnly, Category="Brain")
    TArray<TSubclassOf<UEnemyBrainModule>> moduleClasses;

    /** Runtime instances */
    UPROPERTY(VisibleAnywhere, Transient)
    TArray<UEnemyBrainModule*> moduleInstances;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Brain")
    FEnemyBlackboard blackboard;

    UEnemyBrainComponent();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UFUNCTION(BlueprintPure)
    AEnemyController* GetEnemyController() const { return controller; }

    UFUNCTION(BlueprintPure)
    UStateMachineComponent* GetStateMachine() const { return stateMachineComp; }

    void ActivateBrain();
    void DeactivateBrain();


    UFUNCTION(BlueprintCallable)
    void RequestReevaluate();

    void HandleAnimNotify(FName NotifyName);
    void HandleReceiveHit(const FAtkHitData& HitData);
};