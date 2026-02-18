#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnemyController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSensedDamageSig, AActor*, SourceActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSensedSightSig, AActor*, SeenActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSensedSoundSig, AActor*, HeardActor, FVector, SoundOrigin);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLostSightSig, AActor*, LostActor);

// Broadcast when an EQS query completes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEQSQueryFinishedSig, const FEnvQueryResult&, QueryResult);

// Broadcast when the controller finishes a MoveTo request
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMoveCompletedSig, bool, bSuccess);

/**
 * 
 */
UCLASS()
class HACK_N_SLASH_API AEnemyController : public AAIController
{
	GENERATED_BODY()

private:
	UPROPERTY()
	class AEnemyBase* ownerEnemy;

    void OnEQSQueryFinished(TSharedPtr<FEnvQueryResult> Result);

protected:
	UPROPERTY(EditAnywhere)
	bool bDebugMode {false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UAIPerceptionComponent* aiPercComp;

	//virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	UFUNCTION()
	void SenseUpdated(AActor* SensedActor, FAIStimulus Stimulus);

public:
    FOnSensedDamageSig OnSensedDamageDel;
    FOnSensedSightSig OnSensedSightDel;
    FOnSensedSoundSig OnSensedSoundDel;
    FOnLostSightSig OnLostSightDel;
    FOnMoveCompletedSig OnMoveCompletedDel;
    FOnEQSQueryFinishedSig OnEQSQueryFinishedDel;

	AEnemyController();
	
	/** Run an EQS query template (owner pawn is used as querier). Broadcasts OnEQSQueryFinished when done. */
    void RunEQSQueryHNS(UEnvQuery* QueryTemplate, EEnvQueryRunMode::Type RunMode = EEnvQueryRunMode::SingleResult);

    FAIRequestID MoveToActorHNS(AActor* TargetActor, float AcceptanceRadius = 10.f);
    FAIRequestID MoveToLocationHNS(FVector Location, float AcceptanceRadius = 150.f);
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

	void SetFocusHNS(AActor* Target);
};