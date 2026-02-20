#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
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
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite) float LastSeenTime = -1.f;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FEnvQueryResult ENVQ_Result;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HACK_N_SLASH_API UEnemyBrainComponent : public UActorComponent
{
    GENERATED_BODY()

private:
    /** Currently active module (exclusive control) */
    UPROPERTY()
    UEnemyBrainModule* ActiveModule = nullptr;

    FTimerHandle TH_ActiveModuleExpiry; /** Timer handle for active module expiry (claim duration) */
    FTimerHandle TH_Reeval;

    void InitializeModules();
    void EvaluateModules(const FString& Reason);

    /** Activate / Deactivate */
    void ActivateModule(UEnemyBrainModule* Module);
    void DeactivateModule(UEnemyBrainModule* Module);

    /** Helpers */
    AEnemyController* GetEnemyController() const { return Controller; }

    // Event Handlers bound to controller
    UFUNCTION() void HandleSensedSight(AActor* SeenActor);
    UFUNCTION() void HandleLostSight(AActor* LostActor);
    UFUNCTION() void HandleSensedDamage(AActor* SourceActor);
    UFUNCTION() void HandleSensedSound(AActor* HeardActor, FVector SoundOrigin);
    UFUNCTION() void HandleEQSQueryFinished(const FEnvQueryResult& Result);
    UFUNCTION() void HandleMoveCompleted(bool bSuccess);

protected:
    UPROPERTY(EditAnywhere, Category = "AI")
    bool bDebug { false };

    UPROPERTY() AEnemyController* Controller = nullptr;

public:
    /** Blueprint-configurable list of Module classes */
    UPROPERTY(EditDefaultsOnly, Category="Brain")
    TArray<TSubclassOf<UEnemyBrainModule>> ModuleClasses;

    /** Runtime instances */
    UPROPERTY(Transient)
    TArray<UEnemyBrainModule*> ModuleInstances;

    /** Shared blackboard for modules */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Brain")
    FEnemyBlackboard Blackboard;

    /** Small optional evaluation tick if needed (default 0 = disabled) */
    UPROPERTY(EditDefaultsOnly, Category="Brain")
    float ReevaluateIntervalSeconds = 0.f;

    UEnemyBrainComponent();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    void HandleAnimNotify(FName NotifyName);

    /** Manual re-eval trigger */
    UFUNCTION(BlueprintCallable, Category="Brain")
    void RequestReevaluate();
};