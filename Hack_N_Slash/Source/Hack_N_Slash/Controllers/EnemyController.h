#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "Navigation/PathFollowingComponent.h"
#include "EnemyController.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSensedDamage, AActor*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSensedSight, AActor*);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSensedSound, AActor*, const FVector&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnLostSight, AActor*);

// EQS query finished
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEQSQueryFinished, const FEnvQueryResult&);

// Move completed
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnMoveCompleted, FAIRequestID, EPathFollowingResult::Type);

class AEnemyBase;
class UEnemyBrainComponent;

/**
 * 
 */
UCLASS()
class HACK_N_SLASH_API AEnemyController : public AAIController
{
	GENERATED_BODY()

private:
	UPROPERTY(Transient) AEnemyBase* ownerEnemy;
	UPROPERTY(Transient) UEnemyBrainComponent* enemyBrain;

    void OnEQSQueryFinished(TSharedPtr<FEnvQueryResult> Result);

protected:
	UPROPERTY(EditAnywhere, Category = "Enemy")
	bool bDebug = false;

	UPROPERTY() class UAIPerceptionComponent* aiPercComp;

	//virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	UFUNCTION() void SenseUpdated(AActor* SensedActor, FAIStimulus Stimulus);

public:
    FOnSensedDamage OnSensedDamage;
    FOnSensedSight OnSensedSight;
    FOnSensedSound OnSensedSound;
    FOnLostSight OnLostSight;
    FOnMoveCompleted OnMoveCompletedDel;
    FOnEQSQueryFinished OnEQSQueryFinishedDel;

	AEnemyController();

	float GetMaxAgeSight() const;

	UFUNCTION(BlueprintPure, Category = "Enemy")
	bool IsActorSeen(AActor* Actor);
	
	/** Run an EQS query template (owner pawn is used as querier). Broadcasts OnEQSQueryFinished when done. */
	UFUNCTION(BlueprintCallable, Category = "Enemy")
    void RunEQSQueryHNS(UEnvQuery* QueryTemplate, TMap<FName, float> QueryParams, EEnvQueryRunMode::Type RunMode = EEnvQueryRunMode::SingleResult);

	UFUNCTION(BlueprintCallable, Category = "Enemy")
    FAIRequestID MoveToActorHNS(AActor* TargetActor, float AcceptanceRadius = 300.f);

	UFUNCTION(BlueprintCallable, Category = "Enemy")
    FAIRequestID MoveToLocationHNS(FVector Location, float AcceptanceRadius = 150.f);

	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void SetFocusHNS(AActor* Target);

	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void ClearFocusHNS();
};