#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyBrainComponent.generated.h"

class AEnemyController;

/**
 */
UCLASS()
class HACK_N_SLASH_API UEnemyBrainComponent : public UActorComponent
{
    GENERATED_BODY()

private:
    // Event Handlers bound to controller
    UFUNCTION() void HandleSensedSight(AActor* SeenActor);

    UFUNCTION() void HandleLostSight(AActor* LostActor);

    UFUNCTION() void HandleSensedDamage(AActor* SourceActor);

    UFUNCTION() void HandleSensedSound(AActor* HeardActor, FVector SoundOrigin);

    UFUNCTION() void HandleEQSQueryFinished(const FEnvQueryResult& Result); // Called when controller broadcasts an EQS query result

    UFUNCTION() void HandleMoveCompleted(bool bSuccess); // Called by controller when a MoveTo completes; bound in BeginPlay
    
protected:
    UPROPERTY(EditAnywhere, Category = "AI")
    bool bDebug { false };

    UPROPERTY() AEnemyController* controller;

public:
    UEnemyBrainComponent();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};