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

#include "../Characters/Enemy/EnemyBase.h"
#include "../Characters/Enemy/EnemyBrainComponent.h"
#include "../Characters/Player/Player_Base.h"

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
    enemyBrain = InPawn ? InPawn->FindComponentByClass<UEnemyBrainComponent>() : nullptr;
    if (aiPercComp) aiPercComp->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyController::SenseUpdated);
}

void AEnemyController::OnUnPossess()
{
    if (aiPercComp) aiPercComp->OnTargetPerceptionUpdated.RemoveDynamic(this, &AEnemyController::SenseUpdated);
    ownerEnemy = nullptr;
    enemyBrain = nullptr;
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
    if (!world || !SensedActor || !Cast<APlayer_Base>(SensedActor)) return;

    ACharacter* sensedChar = Cast<ACharacter>(SensedActor);

    const TSubclassOf<UAISense> SenseClass = UAIPerceptionSystem::GetSenseClassForStimulus(world, Stimulus);

    if (SenseClass == UAISense_Damage::StaticClass() && Stimulus.WasSuccessfullySensed())
    {
        if (bDebugMode) UE_LOG(LogTemp, Warning, TEXT("Damage sensed from %s"), *SensedActor->GetName());
        OnSensedDamage.Broadcast(SensedActor);
    }
    else if (sensedChar && SenseClass == UAISense_Sight::StaticClass())
    {
        if (Stimulus.WasSuccessfullySensed())
        {
            if (bDebugMode) UE_LOG(LogTemp, Warning, TEXT("Sight sensed %s"), *sensedChar->GetName());
            OnSensedSight.Broadcast(sensedChar);
        }
        else
        {
            if (bDebugMode) UE_LOG(LogTemp, Warning, TEXT("Lost sight of %s"), *sensedChar->GetName());
            OnLostSight.Broadcast(sensedChar);
        }
    }
    else if (SensedActor && SenseClass == UAISense_Hearing::StaticClass() && Stimulus.WasSuccessfullySensed())
    {
        if (bDebugMode) UE_LOG(LogTemp, Warning, TEXT("Hearing sensed %s"), *SensedActor->GetName());
        OnSensedSound.Broadcast(SensedActor, Stimulus.StimulusLocation);
    }
}

void AEnemyController::RunEQSQueryHNS(UEnvQuery* QueryTemplate, TMap<FName, float> QueryParams, EEnvQueryRunMode::Type RunMode)
{
    if (!QueryTemplate) return;

    FEnvQueryRequest QueryRequest(QueryTemplate, GetPawn());

    for (const TPair<FName, float>& pair : QueryParams)
    {
        FName key = pair.Key;
        float value = pair.Value;
        QueryRequest.SetFloatParam(key, value);
    }

    QueryRequest.Execute(RunMode, this, &AEnemyController::OnEQSQueryFinished);
}

void AEnemyController::OnEQSQueryFinished(TSharedPtr<FEnvQueryResult> Result)
{
    if (!Result) return;

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
    OnEQSQueryFinishedDel.Broadcast(*Result);
}

FAIRequestID AEnemyController::MoveToActorHNS(AActor* TargetActor, float AcceptanceRadius)
{
    if (!ownerEnemy || !TargetActor) return FAIRequestID::InvalidRequest;
    return MoveToActor(TargetActor, AcceptanceRadius, false);
}

FAIRequestID AEnemyController::MoveToLocationHNS(FVector Location, float AcceptanceRadius)
{
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
    OnMoveCompletedDel.Broadcast(RequestID, Result.Code);
}

void AEnemyController::SetFocusHNS(AActor *Target)
{
    if (Target)
    {
        SetFocus(Target);
        if (enemyBrain) enemyBrain->blackboard.bLockedOn = true;
    }
    else ClearFocus(EAIFocusPriority::Gameplay);
}

void AEnemyController::ClearFocusHNS()
{
    ClearFocus(EAIFocusPriority::Gameplay);
    if (enemyBrain) enemyBrain->blackboard.bLockedOn = false;
}
