#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
//#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnemyBrainComponent.generated.h"

class AEnemyController;
class UEnemyBrainModule;
class UStateMachineComponent;

USTRUCT(BlueprintType)
struct FEnemyBlackboard
{
    GENERATED_BODY()
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite) AActor* TargetActor = nullptr;
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite) FVector LastKnownLocation = FVector::ZeroVector;
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite) AActor* LastDamageSource = nullptr;
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite) FVector HomeLocation = FVector::ZeroVector;
    UPROPERTY(EditInstanceOnly, BlueprintReadOnly) TArray<AActor*> PatrolPoints;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TArray<AActor*> EQS_Actors;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TArray<FVector> EQS_Locs;
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite) float LastSeenTime = -1.f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HACK_N_SLASH_API UEnemyBrainComponent : public UActorComponent
{
    GENERATED_BODY()

private:
    FTimerHandle TH_ActiveModuleExpiry; /** Timer handle for active module expiry (claim duration) */
    FTimerHandle TH_Reeval;

    void InitializeModules();
    void EvaluateModules(const FString& Reason);

    /** Activate / Deactivate */
    void ActivateModule(UEnemyBrainModule* Module);
    UFUNCTION() void OnActiveModuleExpired(UEnemyBrainModule* ExpiredModule);
    void DeactivateModule(UEnemyBrainModule* Module);

    // Event Handlers bound to controller
    UFUNCTION() void HandleSensedSight(AActor* SeenActor);
    UFUNCTION() void HandleLostSight(AActor* LostActor);
    UFUNCTION() void HandleSensedDamage(AActor* SourceActor);
    UFUNCTION() void HandleSensedSound(AActor* HeardActor, const FVector& SoundOrigin);
    UFUNCTION() void HandleEQSQueryFinished(const FEnvQueryResult& Result);
    UFUNCTION() void HandleMoveCompleted(bool bSuccess);

protected:
    UPROPERTY(EditAnywhere, Category = "Brain")
    bool bDebug { false };

    UPROPERTY() AEnemyController* controller {nullptr};
    UPROPERTY() UStateMachineComponent* stateMachineComp {nullptr};

    /** Currently active module (exclusive control) */
    UPROPERTY(VisibleAnywhere, Transient, Category="Brain")
    UEnemyBrainModule* activeModule {nullptr};

    /** Blueprint-configurable list of Module classes */
    UPROPERTY(EditDefaultsOnly, Category="Brain")
    TArray<TSubclassOf<UEnemyBrainModule>> moduleClasses;

    /** Runtime instances */
    UPROPERTY(VisibleAnywhere, Transient, Category="Brain")
    TArray<UEnemyBrainModule*> moduleInstances;

    /** Small optional evaluation tick if needed (default 0 = disabled) */
    UPROPERTY(EditDefaultsOnly, Category="Brain")
    float reevaluateIntervalSeconds {0.0f};

public:
    /** Shared blackboard for modules */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Brain")
    FEnemyBlackboard blackboard;

    UEnemyBrainComponent();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    /** Helpers */
    UFUNCTION(BlueprintPure, Category="Brain")
    AEnemyController* GetEnemyController() const { return controller; }

    UFUNCTION(BlueprintPure, Category="Brain")
    UStateMachineComponent* GetStateMachine() const { return stateMachineComp; }

    void HandleAnimNotify(FName NotifyName);

    /** Manual re-eval trigger */
    UFUNCTION(BlueprintCallable, Category="Brain")
    void RequestReevaluate();
};