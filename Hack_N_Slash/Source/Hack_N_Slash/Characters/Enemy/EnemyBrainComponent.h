// EnemyBrainComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyBrainComponent.generated.h"

UENUM(BlueprintType)
enum class EAIIntent : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Investigate UMETA(DisplayName = "Investigate"),
    Pursue      UMETA(DisplayName = "Pursue"),
    Strafe      UMETA(DisplayName = "Strafe"),
    Attack      UMETA(DisplayName = "Attack"),
    TakeCover   UMETA(DisplayName = "Take Cover")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnIntentChangedSig, EAIIntent, NewIntent);

class AEnemyController;
class UEnvQuery;

/**
 * Simplified Enemy Brain component — single component pattern (C++ core + BP hooks).
 * - Keeps movement/perception authoritative in AEnemyController.
 * - Exposes BlueprintNativeEvent hooks so designers can make different behaviours via BP subclassing.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HACK_N_SLASH_API UEnemyBrainComponent : public UActorComponent
{
    GENERATED_BODY()

private:
    // Cached controller (authority, movement & perception)
    UPROPERTY()
    AEnemyController* Controller { nullptr };

    // Patrol runtime state
    int32 CurrentPatrolIndex = -1;
    int32 PatrolDirection = 1; // 1 forward, -1 backward
    FTimerHandle PatrolWaitTimerHandle;

    // Helpers
    void EnsureControllerCached();
    bool HasAuthority() const;
    void ClearPatrolTimer();

    // Perception handlers bound to controller
    UFUNCTION()
    void HandleSensedSight(AActor* SeenActor);

    UFUNCTION()
    void HandleLostSight(AActor* LostActor);

    UFUNCTION()
    void HandleSensedDamage(AActor* SourceActor);

    UFUNCTION()
    void HandleSensedSound(AActor* HeardActor, FVector SoundOrigin);

protected:
    // Debug
    UPROPERTY(EditAnywhere, Category = "AI")
    bool bDebug { false };

    // Exposed state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    EAIIntent CurrentIntent { EAIIntent::Idle };

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    AActor* CurrentTarget { nullptr };

    // Patrol settings exposed to designers
    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "AI|Patrol")
    TArray<AActor*> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Patrol")
    float PatrolWaitMin { 0.5f };

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Patrol")
    float PatrolWaitMax { 2.0f };

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Patrol")
    float PatrolAcceptanceRadius { 50.f };

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Patrol")
    bool bLoopPatrol { true };

    // EQS templates (assign in editor per enemy)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI|EQS")
    UEnvQuery* EQS_Investigate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI|EQS")
    UEnvQuery* EQS_Pursue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI|EQS")
    UEnvQuery* EQS_Strafe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI|EQS")
    UEnvQuery* EQS_TakeCover;

    // Blueprint hooks for behaviour: override in BP subclass to implement different enemy behaviours
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Hooks")
    void OnEnterIntent(EAIIntent Intent, AActor* ForTarget);
    virtual void OnEnterIntent_Implementation(EAIIntent Intent, AActor* ForTarget) { /* default no-op */ }

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Hooks")
    void OnExitIntent(EAIIntent Intent);
    virtual void OnExitIntent_Implementation(EAIIntent Intent) { /* default no-op */ }

    // Called in "OnEnterIntent" for relevant intents
    UFUNCTION(BlueprintCallable, Category = "AI|EQS")
    void RunEQSForIntent(EAIIntent Intent);

    // Called when controller broadcasts an EQS query result
    UFUNCTION()
    void HandleEQSQueryFinished(const FEnvQueryResult& Result);

    // Called by controller when a MoveTo completes; bound in BeginPlay.
    UFUNCTION()
    void NotifyMoveCompleted(bool bSuccess);

public:
    // Blueprint designers can react to intent changes
    UPROPERTY(BlueprintAssignable, Category = "AI")
    FOnIntentChangedSig OnIntentChanged;

    UEnemyBrainComponent();

    // Lifecycle
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // Intent API
    UFUNCTION(BlueprintCallable, Category = "AI")
    EAIIntent GetIntent() const { return CurrentIntent; }

    UFUNCTION(BlueprintCallable, Category = "AI")
    AActor* GetCurrentTarget() const { return CurrentTarget; }

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetIntent(EAIIntent NewIntent, AActor* NewTarget = nullptr);

    // Patrol API
    UFUNCTION(BlueprintCallable, Category = "AI|Patrol")
    void StartPatrol();

    UFUNCTION(BlueprintCallable, Category = "AI|Patrol")
    void StopPatrol();

    UFUNCTION(BlueprintCallable, Category = "AI|Patrol")
    void GoToNextPatrolPoint();
};