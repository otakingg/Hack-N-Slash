#include "EnemyBrainComponent.h"
#include "Modules/EnemyBrainModule.h"
#include "Controllers/EnemyController.h"
#include "../StateMachineComponent.h"

UEnemyBrainComponent::UEnemyBrainComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UEnemyBrainComponent::BeginPlay()
{
    Super::BeginPlay();

    // Controller lookup — prefer the Pawn controller
    APawn* PawnOwner = Cast<APawn>(GetOwner());
    controller = PawnOwner ? Cast<AEnemyController>(PawnOwner->GetController()) : nullptr;
    stateMachineComp = PawnOwner ? PawnOwner->FindComponentByClass<UStateMachineComponent>() : nullptr;

    InitializeModules();

    if (controller)
    {
        controller->OnSensedSightDel.AddDynamic(this, &UEnemyBrainComponent::HandleSensedSight);
        controller->OnLostSightDel.AddDynamic(this, &UEnemyBrainComponent::HandleLostSight);
        controller->OnSensedDamageDel.AddDynamic(this, &UEnemyBrainComponent::HandleSensedDamage);
        controller->OnSensedSoundDel.AddDynamic(this, &UEnemyBrainComponent::HandleSensedSound);
        controller->OnEQSQueryFinishedDel.AddDynamic(this, &UEnemyBrainComponent::HandleEQSQueryFinished);
        controller->OnMoveCompletedDel.AddDynamic(this, &UEnemyBrainComponent::HandleMoveCompleted);
    }

    if (reevaluateIntervalSeconds > 0.f) GetWorld()->GetTimerManager().SetTimer(TH_Reeval, this, &UEnemyBrainComponent::RequestReevaluate, reevaluateIntervalSeconds, true);
    EvaluateModules(TEXT("Begin Play"));
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

    GetWorld()->GetTimerManager().ClearTimer(TH_Reeval);
    GetWorld()->GetTimerManager().ClearTimer(TH_ActiveModuleExpiry);
    Super::EndPlay(EndPlayReason);
}

void UEnemyBrainComponent::InitializeModules()
{
    moduleInstances.Empty();

    for (const TSubclassOf<UEnemyBrainModule>& Cls : moduleClasses)
    {
        if (!Cls) continue;

        UEnemyBrainModule* Inst = NewObject<UEnemyBrainModule>(this, Cls.Get());
        if (!Inst) continue;

        Inst->Initialize(this);
        moduleInstances.Add(Inst);
    }

    // Remove any nulls just in case, then sort by priority descending
    moduleInstances.RemoveAll([](UEnemyBrainModule* M) { return M == nullptr; });
    moduleInstances.Sort([](const UEnemyBrainModule& A, const UEnemyBrainModule& B) { return static_cast<int>(A.priority) > static_cast<int>(B.priority); });
}

void UEnemyBrainComponent::EvaluateModules(const FString& Reason)
{
    for (UEnemyBrainModule* M : moduleInstances)
    {
        if (!M || !M->CanStart(Reason)) continue;

        if (activeModule)
        {
            if (activeModule->CanBeInterruptedBy(M)) {DeactivateModule(activeModule);}
            else continue;
        }
        ActivateModule(M); return;
    }
}
void UEnemyBrainComponent::ActivateModule(UEnemyBrainModule* Module)
{
    if (!Module) return;
    if (activeModule) DeactivateModule(activeModule);
    activeModule = Module;
    activeModule->OnEnter();

    if (activeModule->claimDuration > 0.f)
    {
        GetWorld()->GetTimerManager().ClearTimer(TH_ActiveModuleExpiry);
        FTimerDelegate Del = FTimerDelegate::CreateUObject(this, &UEnemyBrainComponent::OnActiveModuleExpired, activeModule);
        GetWorld()->GetTimerManager().SetTimer(TH_ActiveModuleExpiry, Del, activeModule->claimDuration, false);
    }
}

void UEnemyBrainComponent::OnActiveModuleExpired(UEnemyBrainModule* ExpiredModule)
{
    if (activeModule && activeModule == ExpiredModule)
    {
        DeactivateModule(ExpiredModule);
        RequestReevaluate();
    }
}

void UEnemyBrainComponent::DeactivateModule(UEnemyBrainModule* Module)
{
    if (!Module) return;
    Module->OnExit();
    if (activeModule == Module)
    {
        activeModule = nullptr;
        GetWorld()->GetTimerManager().ClearTimer(TH_ActiveModuleExpiry);
    }
}

void UEnemyBrainComponent::RequestReevaluate() { EvaluateModules(TEXT("Reevaluate")); }

/* ---------------- Event Handlers ---------------- */
void UEnemyBrainComponent::HandleSensedSight(AActor* Seen)
{
    blackboard.TargetActor = Seen;
    blackboard.LastSeenTime = GetWorld() ? GetWorld()->GetTimeSeconds() : -1.f;

    if (activeModule) activeModule->HandleSensedSight(Seen);
    for (UEnemyBrainModule* M : moduleInstances) if (M && M != activeModule) M->HandleSensedSight(Seen);
    EvaluateModules(TEXT("Sight"));
}

void UEnemyBrainComponent::HandleLostSight(AActor* Lost)
{
    if (activeModule) activeModule->HandleLostSight(Lost);
    for (UEnemyBrainModule* M : moduleInstances) if (M && M != activeModule) M->HandleLostSight(Lost);
    EvaluateModules(TEXT("LostSight"));
}

void UEnemyBrainComponent::HandleSensedDamage(AActor* Source)
{
    blackboard.LastDamageSource = Source;
    if (activeModule) activeModule->HandleSensedDamage(Source);
    for (UEnemyBrainModule* M : moduleInstances) if (M && M != activeModule) M->HandleSensedDamage(Source);
    EvaluateModules(TEXT("Damage"));
}

void UEnemyBrainComponent::HandleSensedSound(AActor* Heard, const FVector& Origin)
{
    blackboard.LastKnownLocation = Origin;
    if (activeModule) activeModule->HandleSensedSound(Heard, Origin);
    for (UEnemyBrainModule* M : moduleInstances) if (M && M != activeModule) M->HandleSensedSound(Heard, Origin);
    EvaluateModules(TEXT("Sound"));
}

void UEnemyBrainComponent::HandleEQSQueryFinished(const FEnvQueryResult& Result)
{
    Result.GetAllAsActors(blackboard.EQS_Actors);
    Result.GetAllAsLocations(blackboard.EQS_Locs);
    if (activeModule) activeModule->HandleEQSFinished(Result);
    for (UEnemyBrainModule* M : moduleInstances) if (M && M != activeModule) M->HandleEQSFinished(Result);
    EvaluateModules(TEXT("EQS"));
}

void UEnemyBrainComponent::HandleMoveCompleted(bool bSuccess)
{
    if (activeModule) activeModule->HandleMoveCompleted(bSuccess);
    for (UEnemyBrainModule* M : moduleInstances) if (M && M != activeModule) M->HandleMoveCompleted(bSuccess);
    EvaluateModules(TEXT("MoveCompleted"));
}

void UEnemyBrainComponent::HandleAnimNotify(FName NotifyName)
{
    if (activeModule) activeModule->HandleAnimNotify(NotifyName);
    for (UEnemyBrainModule* M : moduleInstances) if (M && M != activeModule) M->HandleAnimNotify(NotifyName);
    EvaluateModules(TEXT("AnimNotify"));
}