#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSensedDamageSig, AActor*, SourceActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSensedSightSig, AActor*, SeenActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSensedSoundSig, AActor*, HeardActor, FVector, SoundOrigin);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLostSightSig, AActor*, LostActor);

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
    UPROPERTY(BlueprintAssignable, Category="AI|Events")
    FOnSensedDamageSig OnSensedDamageDel;

    UPROPERTY(BlueprintAssignable, Category="AI|Events")
    FOnSensedSightSig OnSensedSightDel;

    UPROPERTY(BlueprintAssignable, Category="AI|Events")
    FOnSensedSoundSig OnSensedSoundDel;

    UPROPERTY(BlueprintAssignable, Category="AI|Events")
    FOnLostSightSig OnLostSightDel;

	void FocusTarget(AActor*);


	AEnemyController();
};