#include "EnemyController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionSystem.h"
#include "Perception/AISense_Damage.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "GameFramework/Character.h"
#include "../Characters/Enemy/EnemyBase.h"

//TArray<AActor*> seenActors;
//aiPercComp->GetKnownPerceivedActors(UAISense_Sight::StaticClass(), seenActors);

AEnemyController::AEnemyController()
{
    PrimaryActorTick.bCanEverTick = false;
    aiPercComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AI Perception"));
    SetPerceptionComponent(*aiPercComp);
}

void AEnemyController::OnPossess(APawn *InPawn)
{
    Super::OnPossess(InPawn);

    ownerEnemy = Cast<AEnemyBase>(InPawn);
    if (aiPercComp) aiPercComp->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyController::SenseUpdated);
}

void AEnemyController::OnUnPossess()
{
    if (aiPercComp) aiPercComp->OnTargetPerceptionUpdated.RemoveDynamic(this, &AEnemyController::SenseUpdated);
    ownerEnemy = nullptr;
    Super::OnUnPossess();
}

void AEnemyController::SenseUpdated(AActor *SensedActor, FAIStimulus Stimulus)
{
    UWorld* world = GetWorld();
    if (!SensedActor || !world) return;

    ACharacter* sensedChar {Cast<ACharacter>(SensedActor)};

    const TSubclassOf<UAISense> SenseClass = UAIPerceptionSystem::GetSenseClassForStimulus(world, Stimulus);

    if (SenseClass == UAISense_Damage::StaticClass() && Stimulus.WasSuccessfullySensed())
    {
        if (bDebugMode) UE_LOG(LogTemp, Warning, TEXT("Damage sensed from %s"), *SensedActor->GetName());
        OnSensedDamageDel.Broadcast(SensedActor);
    }
    else if (sensedChar && SenseClass == UAISense_Sight::StaticClass())
    {
        if (Stimulus.WasSuccessfullySensed())
        {
            if (bDebugMode) UE_LOG(LogTemp, Warning, TEXT("Sight sensed %s"), *sensedChar->GetName());
            OnSensedSightDel.Broadcast(sensedChar);
        }
        else
        {
            if (bDebugMode) UE_LOG(LogTemp, Warning, TEXT("Lost sight of %s"), *sensedChar->GetName());
            OnLostSightDel.Broadcast(sensedChar);
        }
    }
    else if (SensedActor && SenseClass == UAISense_Hearing::StaticClass() && Stimulus.WasSuccessfullySensed())
    {
        if (bDebugMode) UE_LOG(LogTemp, Warning, TEXT("Hearing sensed %s"), *SensedActor->GetName());
        OnSensedSoundDel.Broadcast(SensedActor, Stimulus.StimulusLocation);
    }
}

void AEnemyController::FocusTarget(AActor* Target)
{
    if (Target) SetFocus(Target);
    else ClearFocus(EAIFocusPriority::Gameplay);
}