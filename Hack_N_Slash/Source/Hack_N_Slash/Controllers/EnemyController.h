#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "Navigation/PathFollowingComponent.h"
#include "EnemyController.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSensedDamageSig, AActor*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSensedSightSig, AActor*);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSensedSoundSig, AActor*, const FVector&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnLostSightSig, AActor*);

// EQS query finished
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEQSQueryFinishedSig, const FEnvQueryResult&);

// Move completed
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnMoveCompletedSig, FAIRequestID, EPathFollowingResult::Type);

class AEnemyBase;

/**
 * 
 */
UCLASS()
class HACK_N_SLASH_API AEnemyController : public AAIController
{
	GENERATED_BODY()

private:
	UPROPERTY(Transient) AEnemyBase* ownerEnemy;

    void OnEQSQueryFinished(TSharedPtr<FEnvQueryResult> Result);

protected:
	UPROPERTY(EditAnywhere, Category = "Enemy")
	bool bDebugMode = false;

	UPROPERTY() class UAIPerceptionComponent* aiPercComp;

	//virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	UFUNCTION() void SenseUpdated(AActor* SensedActor, FAIStimulus Stimulus);

public:
    FOnSensedDamageSig OnSensedDamageDel;
    FOnSensedSightSig OnSensedSightDel;
    FOnSensedSoundSig OnSensedSoundDel;
    FOnLostSightSig OnLostSightDel;
    FOnMoveCompletedSig OnMoveCompletedDel;
    FOnEQSQueryFinishedSig OnEQSQueryFinishedDel;

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
};