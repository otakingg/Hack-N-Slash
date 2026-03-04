#include "EnemyController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionSystem.h"
#include "Perception/AISense_Damage.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "GameFramework/Character.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "Navigation/PathFollowingComponent.h"
#include "../Characters/Enemy/EnemyBase.h"
#include "../Interfaces/PlayerInt.h"

AEnemyController::AEnemyController()
{
    PrimaryActorTick.bCanEverTick = true;
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

float AEnemyController::GetMaxAgeSight() const
{
    // Get Sense ID properly
    FAISenseID sightID = UAISense::GetSenseID(UAISense_Sight::StaticClass());

    if (!sightID.IsValid()) return 0.0f;

    // Get config using ID
    const UAISenseConfig* config = aiPercComp->GetSenseConfig(sightID);
    const UAISenseConfig_Sight* sightConfig = Cast<UAISenseConfig_Sight>(config);
    return sightConfig->GetMaxAge();
}

bool AEnemyController::IsActorSeen(AActor* Actor)
{
    if (!Actor) return false;
   TArray<AActor*> seenActors;
   aiPercComp->GetKnownPerceivedActors(UAISense_Sight::StaticClass(), seenActors);
   int result = seenActors.Find(Actor);
   return result != INDEX_NONE;
}

void AEnemyController::SenseUpdated(AActor *SensedActor, FAIStimulus Stimulus)
{
    UWorld* world = GetWorld();
    if (!world || !SensedActor || !SensedActor->Implements<UPlayerInt>()) return;

    ACharacter* sensedChar {Cast<ACharacter>(SensedActor)};

    const TSubclassOf<UAISense> SenseClass = UAIPerceptionSystem::GetSenseClassForStimulus(world, Stimulus);

    if (SenseClass == UAISense_Damage::StaticClass() && Stimulus.WasSuccessfullySensed())
    {
        UE_LOG(LogTemp, Warning, TEXT("Damage sensed from %s"), *SensedActor->GetName());
        OnSensedDamageDel.Broadcast(SensedActor);
    }
    else if (sensedChar && SenseClass == UAISense_Sight::StaticClass())
    {
        if (Stimulus.WasSuccessfullySensed())
        {
            UE_LOG(LogTemp, Warning, TEXT("Sight sensed %s"), *sensedChar->GetName());
            OnSensedSightDel.Broadcast(sensedChar);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Lost sight of %s"), *sensedChar->GetName());
            OnLostSightDel.Broadcast(sensedChar);
        }
    }
    else if (SensedActor && SenseClass == UAISense_Hearing::StaticClass() && Stimulus.WasSuccessfullySensed())
    {
        UE_LOG(LogTemp, Warning, TEXT("Hearing sensed %s"), *SensedActor->GetName());
        OnSensedSoundDel.Broadcast(SensedActor, Stimulus.StimulusLocation);
    }
}

void AEnemyController::RunEQSQueryHNS(UEnvQuery* QueryTemplate, EEnvQueryRunMode::Type RunMode)
{
    if (!QueryTemplate) return;

    FEnvQueryRequest QueryRequest(QueryTemplate, GetPawn());

    QueryRequest.Execute(RunMode, this, &AEnemyController::OnEQSQueryFinished);
}

void AEnemyController::OnEQSQueryFinished(TSharedPtr<FEnvQueryResult> Result)
{
    if (!Result) return;
    OnEQSQueryFinishedDel.Broadcast(*Result);

    if (bDebugMode)
    {
        if (Result->IsSuccessful())
        {
            TArray<FVector> Locs;
            Result->GetAllAsLocations(Locs);
            TArray<AActor*> Actors;
            Result->GetAllAsActors(Actors);
            UE_LOG(LogTemp, Log, TEXT("EQS results: %d locs, %d actors"), Locs.Num(), Actors.Num());
        }
        else UE_LOG(LogTemp, Warning, TEXT("EQS query valid, but failed"));
    }
}

FAIRequestID AEnemyController::MoveToActorHNS(AActor *TargetActor, float AcceptanceRadius)
{
    if (!ownerEnemy || !TargetActor) return FAIRequestID::InvalidRequest;

    FAIRequestID Req = MoveToActor(TargetActor, AcceptanceRadius, false);
    return Req;
}

FAIRequestID AEnemyController::MoveToLocationHNS(FVector Location, float AcceptanceRadius)
{
    // Only allow server-authoritative movement
    if (!ownerEnemy) return FAIRequestID::InvalidRequest;

    return MoveToLocation(Location, AcceptanceRadius, false, true);
}

void AEnemyController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
    Super::OnMoveCompleted(RequestID, Result);

    const bool bSuccess = Result.IsSuccess();
    if (bDebugMode)
    {
        UE_LOG(LogTemp, Warning, TEXT("Move completed with result: %s"), bSuccess ? TEXT("Success") : TEXT("Failure"));
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, FString::Printf(TEXT("Move completed with result: %s"), bSuccess ? TEXT("Success") : TEXT("Failure")));
    }
    OnMoveCompletedDel.Broadcast(bSuccess);
}

void AEnemyController::SetFocusHNS(AActor *Target)
{
    if (Target) SetFocus(Target);
    else ClearFocus(EAIFocusPriority::Gameplay);
}