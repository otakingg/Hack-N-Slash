#include "StateMachineComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../Interfaces/LocomotionCmdInterface.h"
#include "../Interfaces/CombatCmdInterface.h"

UStateMachineComponent::UStateMachineComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UStateMachineComponent::BeginPlay()
{
    Super::BeginPlay();

    ownerChar = Cast<ACharacter>(GetOwner());

    InitializeMovementMap();
    InitializeActionMap();
    CacheCommandInterfaces();

    if (ownerChar)
    {
        ownerChar->LandedDelegate.AddDynamic(this, &UStateMachineComponent::HandleLanded);
        ownerChar->MovementModeChangedDelegate.AddDynamic(this, &UStateMachineComponent::HandleMovementModeChanged);
        ownerChar->OnReachedJumpApex.AddDynamic(this, &UStateMachineComponent::HandleJumpApexReached);
    }

    ApplyBaselineMovement(true);

    if (!currentActionState && defaultActionStateClass)
    {
        if (UActionState* Found = GetActionState(defaultActionStateClass)) ChangeActionState(Found, true);
        else UE_LOG(LogTemp, Warning, TEXT("[%s] Default Action State not registered: %s"), *GetNameSafe(this), *GetNameSafe(defaultActionStateClass.Get()));
    }
}

void UStateMachineComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (ownerChar)
    {
        ownerChar->LandedDelegate.RemoveDynamic(this, &UStateMachineComponent::HandleLanded);
        ownerChar->MovementModeChangedDelegate.RemoveDynamic(this, &UStateMachineComponent::HandleMovementModeChanged);
        ownerChar->OnReachedJumpApex.RemoveDynamic(this, &UStateMachineComponent::HandleJumpApexReached);
    }

    Super::EndPlay(EndPlayReason);
}

// ---------------- Initialization (maps) ----------------
void UStateMachineComponent::InitializeMovementMap()
{
    movementStateInstances.Empty();

    for (const TSubclassOf<UMovementState>& StateClass : movementStateClasses)
    {
        UClass* ClassKey = StateClass.Get();
        if (!ClassKey || ClassKey->HasAnyClassFlags(EClassFlags::CLASS_Abstract)) continue;

        if (movementStateInstances.Contains(ClassKey)) continue; // prevent duplicates

        UMovementState* Instance = NewObject<UMovementState>(this, ClassKey);
        if (!Instance) continue;

        Instance->Initialize(this, ownerChar);
        movementStateInstances.Add(ClassKey, Instance);
    }
}

void UStateMachineComponent::InitializeActionMap()
{
    actionStateInstances.Empty();

    for (const TSubclassOf<UActionState>& StateClass : actionStateClasses)
    {
        UClass* ClassKey = StateClass.Get();
        if (!ClassKey || ClassKey->HasAnyClassFlags(EClassFlags::CLASS_Abstract)) continue;

        if (actionStateInstances.Contains(ClassKey)) continue;

        UActionState* Instance = NewObject<UActionState>(this, ClassKey);
        if (!Instance) continue;

        Instance->Initialize(this, ownerChar);
        actionStateInstances.Add(ClassKey, Instance);
    }
}

/* ---------------- NEW: cache interfaces ---------------- */
void UStateMachineComponent::CacheCommandInterfaces()
{
    if (!ownerChar) return;

    LocomotionCmd = nullptr;
    CombatCmd = nullptr;

    // Find first component implementing locomotion interface
    {
        TArray<UActorComponent*> Comps = ownerChar->GetComponentsByInterface(ULocomotionCmdInterface::StaticClass());
        if (Comps.Num() > 0) LocomotionCmd = Cast<ILocomotionCmdInterface>(Comps[0]);
    }

    // Find first component implementing combat interface
    {
        TArray<UActorComponent*> Comps = ownerChar->GetComponentsByInterface(UCombatCmdInterface::StaticClass());
        if (Comps.Num() > 0) CombatCmd = Cast<ICombatCmdInterface>(Comps[0]);
    }
}

ILocomotionCmdInterface* UStateMachineComponent::GetLocomotionCommands() const { return LocomotionCmd; }

ICombatCmdInterface* UStateMachineComponent::GetCombatCommands() const { return CombatCmd; }

// ---------------- State Lookup ----------------

UActionState* UStateMachineComponent::GetActionState(TSubclassOf<UActionState> StateClass) const
{
    UClass* ClassKey = StateClass.Get();
    if (!ClassKey) return nullptr;

    if (const TObjectPtr<UActionState>* Found = actionStateInstances.Find(ClassKey)) return Found->Get();

    return nullptr;
}

UMovementState* UStateMachineComponent::GetMovementState(TSubclassOf<UMovementState> StateClass) const
{
    UClass* ClassKey = StateClass.Get();
    if (!ClassKey) return nullptr;

    if (const TObjectPtr<UMovementState>* Found = movementStateInstances.Find(ClassKey)) return Found->Get();

    return nullptr;
}

// ---------------- Tag Queries ----------------

bool UStateMachineComponent::IsInMovementTag(FGameplayTag Tag) const { return currentMovementState && currentMovementState->GetStateTag().MatchesTag(Tag); }

bool UStateMachineComponent::IsInActionTag(FGameplayTag Tag) const { return currentActionState && currentActionState->GetStateTag().MatchesTag(Tag); }

bool UStateMachineComponent::IsInAnyTag(FGameplayTag Tag) const
{
    // Action layer overrides movement
    if (IsInActionTag(Tag)) return true;
    return IsInMovementTag(Tag);
}


/* ---------------- Transition Rules (unchanged) ---------------- */

bool UStateMachineComponent::CanTransition(const UCharacterState* Current, const UCharacterState* Next, bool bForce)
{
    if (!Next || Next == Current) return false;
    if (bForce) return true;

    if (Current && !Current->CanExitState()) return false;
    if (!Next->CanEnterState(Current)) return false;
    if (Current && !Current->CanBeInterruptedBy(Next)) return false;

    return true;
}

/* ---------------- Baseline movement (unchanged) ---------------- */

void UStateMachineComponent::ApplyBaselineMovement(bool bForce)
{
    if (!ownerChar) return;

    UCharacterMovementComponent* MoveComp = ownerChar->GetCharacterMovement();
    if (!MoveComp) return;

    const bool bGrounded = MoveComp->IsMovingOnGround();
    TSubclassOf<UMovementState> DesiredClass = bGrounded ? defaultGroundMovementClass : defaultAirMovementClass;

    if (!DesiredClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] ApplyBaselineMovement: Desired baseline class is not set."), *GetNameSafe(this));
        return;
    }

    UMovementState* Desired = GetMovementState(DesiredClass);
    if (!Desired)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] ApplyBaselineMovement: Baseline not registered: %s"),
               *GetNameSafe(this), *GetNameSafe(DesiredClass.Get()));
        return;
    }

    if (!bForce && currentMovementState == Desired) return;
    ChangeMovementState(Desired, bForce);
}

/* ---------------- State changes (unchanged) ---------------- */

void UStateMachineComponent::ChangeState(EStateLayer Layer, UCharacterState* NewState, bool bForce)
{
    if (Layer == EStateLayer::Movement) ChangeMovementState(Cast<UMovementState>(NewState), bForce);
    else                               ChangeActionState(Cast<UActionState>(NewState), bForce);
}

void UStateMachineComponent::ChangeMovementState(UMovementState* NewState, bool bForce)
{
    if (!CanTransition(currentMovementState, NewState, bForce)) return;

    if (currentMovementState) currentMovementState->ExitState();
    previousMovementState = currentMovementState;
    currentMovementState = NewState;
    currentMovementState->EnterState();
}

void UStateMachineComponent::ChangeActionState(UActionState* NewState, bool bForce)
{
    if (!CanTransition(currentActionState, NewState, bForce)) return;

    if (currentActionState) currentActionState->ExitState();
    previousActionState = currentActionState;
    currentActionState = NewState;
    currentActionState->EnterState();
}

void UStateMachineComponent::RequestAirMode(TSubclassOf<UAirborneModeState> ModeClass)
{
    if (UAirContainerState* Air = Cast<UAirContainerState>(currentMovementState)) Air->RequestAirborneMode(ModeClass);
}

void UStateMachineComponent::ClearAirMode()
{
    if (UAirContainerState* Air = Cast<UAirContainerState>(currentMovementState)) Air->ClearAirMode();
}

/* ---------------- Unified Requests (NEW) ----------------
   NOTE: For now, these call your existing state methods (OnInputX).
   Next step will be migrating state method names/signatures to OnMoveIntent / etc + Context.
*/

static FCommandContext MakeDefaultCtx(UObject* Instigator, ECommandSource Source)
{
    FCommandContext C;
    C.Source = Source;
    C.Instigator = Instigator;

    if (Instigator)
    {
        if (UWorld* World = Instigator->GetWorld()) C.TimestampSeconds = World->GetTimeSeconds();
    }
    return C;
}

/* ---------------- Unified Requests ---------------- */

void UStateMachineComponent::RequestAttack(const FVector2D& InputVector, const FCommandContext& Ctx)
{
    // Action-layer concern (typically)
    if (currentActionState) currentActionState->OnAttackPressed(InputVector, Ctx);
}

void UStateMachineComponent::RequestJumpPressed(const FCommandContext& Ctx)
{
    const bool bConsumed = (currentActionState && currentActionState->OnJumpPressed(Ctx));
    if (!bConsumed && currentMovementState) currentMovementState->OnJumpPressed(Ctx);
}

void UStateMachineComponent::RequestJumpReleased(const FCommandContext& Ctx)
{
    const bool bConsumed = (currentActionState && currentActionState->OnJumpReleased(Ctx));
    if (!bConsumed && currentMovementState) currentMovementState->OnJumpReleased(Ctx);
}

void UStateMachineComponent::RequestLook(const FVector2D& InputVector, const FCommandContext& Ctx)
{
    const bool bConsumed = (currentActionState && currentActionState->OnLookIntent(InputVector, Ctx));
    if (!bConsumed && currentMovementState) currentMovementState->OnLookIntent(InputVector, Ctx);
}

void UStateMachineComponent::RequestMove(const FVector2D& InputVector, const FCommandContext& Ctx)
{
    const bool bConsumed = (currentActionState && currentActionState->OnMoveIntent(InputVector, Ctx));
    if (!bConsumed && currentMovementState) currentMovementState->OnMoveIntent(InputVector, Ctx);
}

/* ---------------- Compatibility Input Adapters ---------------- */

void UStateMachineComponent::OnInputAttackPressed(const FVector2D& InputVector)
{
    RequestAttack(InputVector, MakeDefaultCtx(ownerChar, ECommandSource::Player));
}

void UStateMachineComponent::OnInputJumpPressed()
{
    RequestJumpPressed(MakeDefaultCtx(ownerChar, ECommandSource::Player));
}

void UStateMachineComponent::OnInputJumpReleased()
{
    RequestJumpReleased(MakeDefaultCtx(ownerChar, ECommandSource::Player));
}

void UStateMachineComponent::OnInputLook(const FVector2D& InputVector)
{
    RequestLook(InputVector, MakeDefaultCtx(ownerChar, ECommandSource::Player));
}

void UStateMachineComponent::OnInputMove(const FVector2D& InputVector)
{
    RequestMove(InputVector, MakeDefaultCtx(ownerChar, ECommandSource::Player));
}

/* ---------------- Character / Anim forwarding (unchanged) ---------------- */

void UStateMachineComponent::HandleJumpApexReached()
{
    if (currentMovementState) currentMovementState->OnJumpApexReached();
}

void UStateMachineComponent::HandleLanded(const FHitResult& Hit)
{
    ApplyBaselineMovement(false);
    if (currentMovementState) currentMovementState->OnLanded(Hit);
}

void UStateMachineComponent::HandleMovementModeChanged(ACharacter* InCharacter, EMovementMode PrevMovementMode, uint8 PrevCustomMode)
{
    ApplyBaselineMovement(false);
    if (currentMovementState) currentMovementState->OnMovementModeChanged(InCharacter, PrevMovementMode, PrevCustomMode);
}

void UStateMachineComponent::OnAnimNotify(FName NotifyName)
{
    if (currentActionState)   currentActionState->OnAnimNotify(NotifyName);
    if (currentMovementState) currentMovementState->OnAnimNotify(NotifyName);
}

void UStateMachineComponent::OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
    if (currentActionState)   currentActionState->OnMontageBlendingOut(Montage, bInterrupted);
    if (currentMovementState) currentMovementState->OnMontageBlendingOut(Montage, bInterrupted);
}