#include "EnemyBrainComponent.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "Controllers/EnemyController.h"

UEnemyBrainComponent::UEnemyBrainComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UEnemyBrainComponent::BeginPlay()
{
    Super::BeginPlay();

    EnsureControllerCached();

    if (Controller)
    {
        // Bind perception & movement completion delegates
        Controller->OnSensedSightDel.AddDynamic(this, &UEnemyBrainComponent::HandleSensedSight);
        Controller->OnLostSightDel.AddDynamic(this, &UEnemyBrainComponent::HandleLostSight);
        Controller->OnSensedDamageDel.AddDynamic(this, &UEnemyBrainComponent::HandleSensedDamage);
        Controller->OnSensedSoundDel.AddDynamic(this, &UEnemyBrainComponent::HandleSensedSound);
        Controller->OnEQSQueryFinishedDel.AddDynamic(this, &UEnemyBrainComponent::HandleEQSQueryFinished);
        Controller->OnMoveCompletedDel.AddDynamic(this, &UEnemyBrainComponent::NotifyMoveCompleted);
    }

    // Optionally start patrol automatically if points present and we are authoritative
    if (PatrolPoints.Num() > 0 && HasAuthority()) StartPatrol();
}

void UEnemyBrainComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (Controller)
    {
        Controller->OnSensedSightDel.RemoveDynamic(this, &UEnemyBrainComponent::HandleSensedSight);
        Controller->OnLostSightDel.RemoveDynamic(this, &UEnemyBrainComponent::HandleLostSight);
        Controller->OnSensedDamageDel.RemoveDynamic(this, &UEnemyBrainComponent::HandleSensedDamage);
        Controller->OnSensedSoundDel.RemoveDynamic(this, &UEnemyBrainComponent::HandleSensedSound);
        Controller->OnEQSQueryFinishedDel.RemoveDynamic(this, &UEnemyBrainComponent::HandleEQSQueryFinished);
        Controller->OnMoveCompletedDel.RemoveDynamic(this, &UEnemyBrainComponent::NotifyMoveCompleted);
    }

    ClearPatrolTimer();
    Super::EndPlay(EndPlayReason);
}

void UEnemyBrainComponent::EnsureControllerCached()
{
    if (Controller) return;

    if (APawn* P = Cast<APawn>(GetOwner()))
    {
        Controller = Cast<AEnemyController>(P->GetController());
        if (!Controller && bDebug) UE_LOG(LogTemp, Warning, TEXT("[%s] EnemyBrain: Controller not found on pawn"), *GetNameSafe(this));
    }
}

bool UEnemyBrainComponent::HasAuthority() const
{
    const AActor* Owner = GetOwner();
    return Owner ? Owner->HasAuthority() : false;
}

void UEnemyBrainComponent::SetIntent(EAIIntent NewIntent, AActor* NewTarget)
{
    if (!HasAuthority()) return;

    if (CurrentIntent == NewIntent && CurrentTarget == NewTarget) return;

    OnExitIntent(CurrentIntent);

    CurrentIntent = NewIntent;
    CurrentTarget = NewTarget;

    OnIntentChanged.Broadcast(CurrentIntent);
    OnEnterIntent(CurrentIntent, CurrentTarget);
}

void UEnemyBrainComponent::RunEQSForIntent(EAIIntent Intent)
{
    EnsureControllerCached();
    if (!Controller)
    {
        if (bDebug) UE_LOG(LogTemp, Warning, TEXT("RunEQSForIntent: Controller missing"));
        return;
    }

    UEnvQuery* Template = nullptr;
    EEnvQueryRunMode::Type RunMode = EEnvQueryRunMode::AllMatching;

    switch (Intent)
    {
        case EAIIntent::Pursue: Template = EQS_Pursue; RunMode = EEnvQueryRunMode::SingleResult; break;
        case EAIIntent::Investigate: Template = EQS_Investigate; RunMode = EEnvQueryRunMode::SingleResult; break;
        case EAIIntent::TakeCover: Template = EQS_TakeCover; RunMode = EEnvQueryRunMode::RandomBest25Pct; break;
        case EAIIntent::Strafe: Template = EQS_Strafe; RunMode = EEnvQueryRunMode::RandomBest25Pct; break;
        default: break;
    }

    if (!Template)
    {
        if (bDebug) UE_LOG(LogTemp, Warning, TEXT("RunEQSForIntent: no EQS template for intent %d"), (int)Intent);
        return;
    }

    Controller->RunEQSQueryHNS(Template, RunMode);
}

/* ---------- EQS results handler ----------
   This is a very simple selection policy: prefer actors if present, else first location.
   You can replace with distance-based, score-based, random-with-weighting, etc.
*/
void UEnemyBrainComponent::HandleEQSQueryFinished(const FEnvQueryResult& Result)
{
    if (!HasAuthority()) return;

    TArray<FVector> Locs;
    Result.GetAllAsLocations(Locs);
    TArray<AActor*> Actors;
    Result.GetAllAsActors(Actors);

    if (bDebug) UE_LOG(LogTemp, Log, TEXT("Brain: EQS returned %d actors, %d locations"), Actors.Num(), Locs.Num());

    if (Actors.Num() > 0) Controller->MoveToActorHNS(Actors[0], 150.f);
    else if (Locs.Num() > 0)
    {
        // Strafe might prefer alternating sides, so flip PatrolDirection-style boolean
        Controller->MoveToLocationHNS(Locs[0], 50.f);
    }
    else if (bDebug) UE_LOG(LogTemp, Log, TEXT("Brain: EQS returned no usable results"));
}
void UEnemyBrainComponent::StartPatrol()
{
    if (!HasAuthority()) return;
    if (PatrolPoints.Num() == 0)
    {
        if (bDebug) UE_LOG(LogTemp, Warning, TEXT("[%s] StartPatrol called but PatrolPoints is empty."), *GetNameSafe(this));
        return;
    }

    // Initialize patrol index/direction and set intent
    CurrentPatrolIndex = 0;
    PatrolDirection = 1;
    SetIntent(EAIIntent::Patrol, nullptr);

    EnsureControllerCached();
    if (Controller && PatrolPoints.IsValidIndex(CurrentPatrolIndex) && PatrolPoints[CurrentPatrolIndex]) Controller->MoveToActorHNS(PatrolPoints[CurrentPatrolIndex], PatrolAcceptanceRadius);
}

void UEnemyBrainComponent::StopPatrol()
{
    if (!HasAuthority()) return;

    ClearPatrolTimer();
    CurrentPatrolIndex = -1;

    if (Controller) Controller->StopMovement();

    // If currently patrol, go to idle
    if (CurrentIntent == EAIIntent::Patrol) SetIntent(EAIIntent::Idle, nullptr);
}

void UEnemyBrainComponent::GoToNextPatrolPoint()
{
    if (!HasAuthority()) return;
    if (PatrolPoints.Num() == 0) return;

    CurrentPatrolIndex += PatrolDirection;

    // Wrap or ping-pong based on bLoopPatrol
    if (CurrentPatrolIndex >= PatrolPoints.Num())
    {
        if (bLoopPatrol) CurrentPatrolIndex = 0;
        else
        {
            PatrolDirection = -1;
            CurrentPatrolIndex = FMath::Max(0, PatrolPoints.Num() - 2);
        }
    }
    else if (CurrentPatrolIndex < 0)
    {
        if (bLoopPatrol) CurrentPatrolIndex = PatrolPoints.Num() - 1;
        else
        {
            PatrolDirection = 1;
            CurrentPatrolIndex = FMath::Min(1, PatrolPoints.Num() - 1);
        }
    }

    EnsureControllerCached();
    if (Controller && PatrolPoints.IsValidIndex(CurrentPatrolIndex) && PatrolPoints[CurrentPatrolIndex]) Controller->MoveToActorHNS(PatrolPoints[CurrentPatrolIndex], PatrolAcceptanceRadius);
}

void UEnemyBrainComponent::NotifyMoveCompleted(bool bSuccess)
{
    // Called when controller broadcasts movement completion
    // For patrol, start the wait timer then advance
    if (CurrentIntent == EAIIntent::Patrol)
    {
        // Notify BP hook that we arrived (BP may want to do animations / sounds)
        OnEnterIntent(CurrentIntent, CurrentTarget);

        // Start wait timer then advance
        ClearPatrolTimer();
        const float Wait = FMath::FRandRange(PatrolWaitMin, PatrolWaitMax);
        if (GetWorld()) GetWorld()->GetTimerManager().SetTimer(PatrolWaitTimerHandle, this, &UEnemyBrainComponent::GoToNextPatrolPoint, Wait, false);
    }
    else OnEnterIntent(CurrentIntent, CurrentTarget); // Non-patrol movement completion forwarded to BP hook — designers can override OnEnterIntent to handle it
}

void UEnemyBrainComponent::ClearPatrolTimer()
{
    if (GetWorld()) GetWorld()->GetTimerManager().ClearTimer(PatrolWaitTimerHandle);
}

/* ---------------- Perception handlers ---------------- */

void UEnemyBrainComponent::HandleSensedSight(AActor* SeenActor)
{
    if (!HasAuthority() || !SeenActor) return;

    // Default: pursue what we see
    SetIntent(EAIIntent::Pursue, SeenActor);
}

void UEnemyBrainComponent::HandleLostSight(AActor* LostActor)
{
    if (!HasAuthority() || !LostActor) return;

    // If we lost our current target, switch to investigate (you may store last-known-location in future)
    if (CurrentTarget == LostActor) SetIntent(EAIIntent::Investigate, nullptr);
}

void UEnemyBrainComponent::HandleSensedDamage(AActor* SourceActor)
{
    if (!HasAuthority() || !SourceActor) return;

    SetIntent(EAIIntent::Pursue, SourceActor);
}

void UEnemyBrainComponent::HandleSensedSound(AActor* HeardActor, FVector SoundOrigin)
{
    if (!HasAuthority()) return;
    // Default: investigate the sound origin (no direct target)
    // Designers can inspect the controller/perception if they need Stimulus location details.
    SetIntent(EAIIntent::Investigate, nullptr);
}