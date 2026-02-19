#include "EnemyBrainComponent.h"
//#include "EnvironmentQuery/EnvQuery.h"
//#include "EnvironmentQuery/EnvQueryManager.h"
//#include "Kismet/KismetMathLibrary.h"
#include "Controllers/EnemyController.h"

UEnemyBrainComponent::UEnemyBrainComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UEnemyBrainComponent::BeginPlay()
{
    Super::BeginPlay();

    AActor* owner {GetOwner()};
    controller = owner ? Cast<AEnemyController>(owner->GetInstigatorController()) : nullptr;

    if (controller)
    {
        controller->OnSensedSightDel.AddDynamic(this, &UEnemyBrainComponent::HandleSensedSight);
        controller->OnLostSightDel.AddDynamic(this, &UEnemyBrainComponent::HandleLostSight);
        controller->OnSensedDamageDel.AddDynamic(this, &UEnemyBrainComponent::HandleSensedDamage);
        controller->OnSensedSoundDel.AddDynamic(this, &UEnemyBrainComponent::HandleSensedSound);
        controller->OnEQSQueryFinishedDel.AddDynamic(this, &UEnemyBrainComponent::HandleEQSQueryFinished);
        controller->OnMoveCompletedDel.AddDynamic(this, &UEnemyBrainComponent::HandleMoveCompleted);
    }
}

void UEnemyBrainComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (controller)
    {
        controller->OnSensedSightDel.RemoveDynamic(this, &UEnemyBrainComponent::HandleSensedSight);
        controller->OnLostSightDel.RemoveDynamic(this, &UEnemyBrainComponent::HandleLostSight);
        controller->OnSensedDamageDel.RemoveDynamic(this, &UEnemyBrainComponent::HandleSensedDamage);
        controller->OnSensedSoundDel.RemoveDynamic(this, &UEnemyBrainComponent::HandleSensedSound);
        controller->OnEQSQueryFinishedDel.RemoveDynamic(this, &UEnemyBrainComponent::HandleEQSQueryFinished);
        controller->OnMoveCompletedDel.RemoveDynamic(this, &UEnemyBrainComponent::HandleMoveCompleted);
    }

    Super::EndPlay(EndPlayReason);
}

/* ---------------- Event Handlers ---------------- */
void UEnemyBrainComponent::HandleSensedSight(AActor* SeenActor) {}
void UEnemyBrainComponent::HandleLostSight(AActor* LostActor) {}
void UEnemyBrainComponent::HandleSensedDamage(AActor* SourceActor) {}
void UEnemyBrainComponent::HandleSensedSound(AActor* HeardActor, FVector SoundOrigin) {}
void UEnemyBrainComponent::HandleEQSQueryFinished(const FEnvQueryResult &Result) {}
void UEnemyBrainComponent::HandleMoveCompleted(bool bSuccess) {}