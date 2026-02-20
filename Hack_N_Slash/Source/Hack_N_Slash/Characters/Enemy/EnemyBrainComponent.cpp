#include "EnemyBrainComponent.h"
#include "Modules/EnemyBrainModule.h"
#include "Controllers/EnemyController.h"

UEnemyBrainComponent::UEnemyBrainComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UEnemyBrainComponent::BeginPlay()
{
    Super::BeginPlay();

    // Controller lookup — prefer the Pawn controller
    APawn* PawnOwner = Cast<APawn>(GetOwner());
    Controller = PawnOwner ? Cast<AEnemyController>(PawnOwner->GetController()) : nullptr;

    InitializeModules();

    if (Controller)
    {
        Controller->OnSensedSightDel.AddDynamic(this, &UEnemyBrainComponent::HandleSensedSight);
        Controller->OnLostSightDel.AddDynamic(this, &UEnemyBrainComponent::HandleLostSight);
        Controller->OnSensedDamageDel.AddDynamic(this, &UEnemyBrainComponent::HandleSensedDamage);
        Controller->OnSensedSoundDel.AddDynamic(this, &UEnemyBrainComponent::HandleSensedSound);
        Controller->OnEQSQueryFinishedDel.AddDynamic(this, &UEnemyBrainComponent::HandleEQSQueryFinished);
        Controller->OnMoveCompletedDel.AddDynamic(this, &UEnemyBrainComponent::HandleMoveCompleted);
    }

    if (ReevaluateIntervalSeconds > 0.f) GetWorld()->GetTimerManager().SetTimer(TH_Reeval, this, &UEnemyBrainComponent::RequestReevaluate, ReevaluateIntervalSeconds, true);
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
        Controller->OnMoveCompletedDel.RemoveDynamic(this, &UEnemyBrainComponent::HandleMoveCompleted);
    }

    GetWorld()->GetTimerManager().ClearTimer(TH_Reeval);
    GetWorld()->GetTimerManager().ClearTimer(TH_ActiveModuleExpiry);
    Super::EndPlay(EndPlayReason);
}

void UEnemyBrainComponent::InitializeModules()
{
    ModuleInstances.Empty();

    for (const TSubclassOf<UEnemyBrainModule>& Cls : ModuleClasses)
    {
        if (!Cls) continue;

        // Use Cls.Get() for clarity
        UEnemyBrainModule* Inst = NewObject<UEnemyBrainModule>(this, Cls.Get());
        if (!Inst) continue;

        Inst->Initialize(this);
        ModuleInstances.Add(Inst);
    }

    // sort by priority descending so higher priority checked first
    ModuleInstances.Sort([](const UEnemyBrainModule& A, const UEnemyBrainModule& B) {return static_cast<int>(A.priority) > static_cast<int>(B.priority);});
}

void UEnemyBrainComponent::EvaluateModules(const FString& Reason)
{
    // If there's an active module and it still wants to keep control, skip.
    if (ActiveModule)
    {
        // Let active module keep control until it explicitly yields or claim expires.
        return;
    }

    for (UEnemyBrainModule* M : ModuleInstances)
    {
        if (!M) continue;
        if (M->CanStart(Reason))
        {
            ActivateModule(M);
            return;
        }
    }
}

void UEnemyBrainComponent::ActivateModule(UEnemyBrainModule* Module)
{
    if (!Module) return;
    if (ActiveModule) DeactivateModule(ActiveModule);
    ActiveModule = Module;
    ActiveModule->OnEnter();

    // Schedule claim expiry if requested
    if (ActiveModule->claimDuration > 0.f)
    {
        GetWorld()->GetTimerManager().ClearTimer(TH_ActiveModuleExpiry);
        FTimerDelegate ExpiryDel = FTimerDelegate::CreateLambda([this]()
        {
            if (ActiveModule)
            {
                DeactivateModule(ActiveModule);
                RequestReevaluate();
            }
        });
        GetWorld()->GetTimerManager().SetTimer(TH_ActiveModuleExpiry, ExpiryDel, ActiveModule->claimDuration, false);
    }
}

void UEnemyBrainComponent::DeactivateModule(UEnemyBrainModule* Module)
{
    if (!Module) return;
    Module->OnExit();
    if (ActiveModule == Module)
    {
        ActiveModule = nullptr;
        GetWorld()->GetTimerManager().ClearTimer(TH_ActiveModuleExpiry);
    }
}

void UEnemyBrainComponent::RequestReevaluate() { EvaluateModules(TEXT("Reevaluate")); }

/* ---------------- Event Handlers ---------------- */
void UEnemyBrainComponent::HandleSensedSight(AActor* Seen)
{
    Blackboard.TargetActor = Seen;
    Blackboard.LastSeenTime = GetWorld() ? GetWorld()->GetTimeSeconds() : -1.f;

    //for (UEnemyBrainModule* M : ModuleInstances) if (M) M->HandleSensedSight(Seen);
    if (ActiveModule) ActiveModule->HandleSensedSight(Seen);
    EvaluateModules(TEXT("Sight"));
}

void UEnemyBrainComponent::HandleLostSight(AActor* Lost)
{
    //for (UEnemyBrainModule* M : ModuleInstances) if (M) M->HandleLostSight(Lost);
    if (ActiveModule) ActiveModule->HandleLostSight(Lost);
    EvaluateModules(TEXT("LostSight"));
}

void UEnemyBrainComponent::HandleSensedDamage(AActor* Source)
{
    Blackboard.LastDamageSource = Source;
    //for (UEnemyBrainModule* M : ModuleInstances) if (M) M->HandleSensedDamage(Source);
    if (ActiveModule) ActiveModule->HandleSensedDamage(Source);
    EvaluateModules(TEXT("Damage"));
}

void UEnemyBrainComponent::HandleSensedSound(AActor* Heard, FVector Origin)
{
    Blackboard.LastKnownLocation = Origin;
    //for (UEnemyBrainModule* M : ModuleInstances) if (M) M->HandleSensedSound(Heard, Origin);
    if (ActiveModule) ActiveModule->HandleSensedSound(Heard, Origin);
    EvaluateModules(TEXT("Sound"));
}

void UEnemyBrainComponent::HandleEQSQueryFinished(const FEnvQueryResult& Result)
{
    Blackboard.ENVQ_Result = Result;
    //for (UEnemyBrainModule* M : ModuleInstances) if (M) M->HandleEQSFinished(Result);
    if (ActiveModule) ActiveModule->HandleEQSFinished(Result);
    EvaluateModules(TEXT("EQS"));
}

void UEnemyBrainComponent::HandleMoveCompleted(bool bSuccess)
{
    //for (UEnemyBrainModule* M : ModuleInstances) if (M) M->HandleMoveCompleted(bSuccess);
    if (ActiveModule) ActiveModule->HandleMoveCompleted(bSuccess);
    EvaluateModules(TEXT("MoveCompleted"));
}

void UEnemyBrainComponent::HandleAnimNotify(FName NotifyName)
{
    //for (UEnemyBrainModule* M : ModuleInstances) if (M) M->HandleAnimNotify(NotifyName);
    if (ActiveModule) ActiveModule->HandleAnimNotify(NotifyName);
    EvaluateModules(TEXT("AnimNotify"));
}