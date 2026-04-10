#include "StateMachineComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../Tags/CharacterStateTagNamespaces.h"
#include "../Structs/FAtkHitData.h"
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

    CacheCommandInterfaces();
    InitializeMovementMap();
    InitializeActionMap();

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
        else if (bDebug) UE_LOG(LogTemp, Warning, TEXT("[%s] Default Action State not registered: %s"), *GetNameSafe(this), *GetNameSafe(defaultActionStateClass.Get()));
    }

    RebuildActiveStateTags(); // Ensure tags are correct immediately
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

// ---------------------- Cache Interfaces ------------------
void UStateMachineComponent::CacheCommandInterfaces()
{
    if (!ownerChar) return;

    iLocomotionCmd = nullptr;
    iCombatCmd     = nullptr;

    // ---------------- Locomotion ----------------

    TArray<UActorComponent*> LocoComps = ownerChar->GetComponentsByInterface(ULocomotionCmdInterface::StaticClass());
    if (LocoComps.Num() > 0) iLocomotionCmd = Cast<ILocomotionCmdInterface>(LocoComps[0]);
    else if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("StateMachineComp: CacheCmdInterfaces: No locomotion cmd interface found"));

    // ---------------- Combat ----------------

    TArray<UActorComponent*> CombatComps = ownerChar->GetComponentsByInterface(UCombatCmdInterface::StaticClass());
    if (CombatComps.Num() > 0) iCombatCmd = Cast<ICombatCmdInterface>(CombatComps[0]);
    else if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("StateMachineComp: CacheCmdInterfaces: No combat cmd interface found"));
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

ICombatCmdInterface* UStateMachineComponent::GetCombatCommands() const { return iCombatCmd; }
ILocomotionCmdInterface* UStateMachineComponent::GetLocomotionCommands() const { return iLocomotionCmd; }

// ---------------- State Lookup ----------------

UActionState* UStateMachineComponent::GetActionState(TSubclassOf<UActionState> StateClass) const
{
    UClass* ClassKey = StateClass.Get();
    if (!ClassKey) return nullptr;

    if (const TObjectPtr<UActionState>* Found = actionStateInstances.Find(ClassKey)) return Found->Get();

    return nullptr;
}

UActionState* UStateMachineComponent::GetActionStateByTag(const FGameplayTag& Tag) const
{
    if (!Tag.IsValid()) return nullptr;

    for (const auto& Pair : actionStateInstances)
    {
        UActionState* State = Pair.Value;
        if (State && State->HasExactStateTag(Tag)) return State;
    }

    return nullptr;
}

UMovementState* UStateMachineComponent::GetMovementState(TSubclassOf<UMovementState> StateClass) const
{
    UClass* ClassKey = StateClass.Get();
    if (!ClassKey) return nullptr;

    if (const TObjectPtr<UMovementState>* Found = movementStateInstances.Find(ClassKey)) return Found->Get();

    return nullptr;
}

UMovementState* UStateMachineComponent::GetMovementStateByTag(const FGameplayTag& Tag) const
{
    if (!Tag.IsValid()) return nullptr;
    
    for (const auto& Pair : movementStateInstances)
    {
        UMovementState* State = Pair.Value;
        if (State && State->HasExactStateTag(Tag)) return State;
    }

    return nullptr;
}

// ---------------- Tag Queries ----------------
void UStateMachineComponent::RebuildActiveStateTags()
{
    activeStateTags.Reset();

    if (currentMovementState) currentMovementState->GatherStateTags(activeStateTags);
    if (currentActionState) currentActionState->GatherStateTags(activeStateTags);

    if (bDebug)
    {
        const FString MoveStr = currentMovementState ? currentMovementState->GetStateTag().GetTagName().ToString() : TEXT("None");
        const FString ActStr  = currentActionState   ? currentActionState->GetStateTag().GetTagName().ToString() : TEXT("None");

        UE_LOG(LogTemp, Log, TEXT("ActiveStateTags: Move=%s | Action=%s | Count=%d"), *MoveStr, *ActStr, activeStateTags.Num());
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("ActiveStateTags: Move=%s | Action=%s | Count=%d"), *MoveStr, *ActStr, activeStateTags.Num()));
    }
}

bool UStateMachineComponent::HasActiveTag(const FGameplayTag& Tag) const { return activeStateTags.HasTag(Tag); }
bool UStateMachineComponent::HasExactActiveTag(const FGameplayTag& Tag) const { return activeStateTags.HasTagExact(Tag); }
bool UStateMachineComponent::IsInMovementTag(const FGameplayTag& Tag) const { return currentMovementState && currentMovementState->HasStateTag(Tag); }
bool UStateMachineComponent::IsInActionTag(const FGameplayTag& Tag) const { return currentActionState && currentActionState->HasStateTag(Tag); }


/* ---------------- Transition Rules (unchanged) ---------------- */

bool UStateMachineComponent::CanTransition(const UCharacterState* Current, const UCharacterState* Next, bool bForce)
{
    //if (!Next) return false;
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

    UCharacterMovementComponent* moveComp = ownerChar->GetCharacterMovement();
    if (!moveComp) return;

    const bool bGrounded = moveComp->IsMovingOnGround();
    TSubclassOf<UMovementState> desiredClass = bGrounded ? defaultGroundMovementClass : defaultAirMovementClass;

    if (!desiredClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] ApplyBaselineMovement: Desired baseline class is not set."), *GetNameSafe(this));
        return;
    }

    UMovementState* desired = GetMovementState(desiredClass);
    if (!desired)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] ApplyBaselineMovement: Baseline not registered: %s"), *GetNameSafe(this), *GetNameSafe(desiredClass.Get()));
        return;
    }

    if (!bForce && currentMovementState == desired) return; // Avoid pointless re-enter unless forced
    ChangeMovementState(desired, bForce);
}

/* ---------------- State changes ---------------- */

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

    RebuildActiveStateTags();
}

void UStateMachineComponent::ChangeActionState(UActionState* NewState, bool bForce)
{
    if (!CanTransition(currentActionState, NewState, bForce)) return;

    if (currentActionState) currentActionState->ExitState();
    previousActionState = currentActionState;
    currentActionState = NewState;
    currentActionState->EnterState();

    RebuildActiveStateTags();
}

void UStateMachineComponent::ClearActionState()
{
    UActionState* noneState = GetActionStateByTag(ActionTags::None);
    ChangeActionState(noneState, true);
}

void UStateMachineComponent::RequestAirborneMode(TSubclassOf<UAirborneModeState> ModeClass)
{
    if (UAirContainerState* Air = Cast<UAirContainerState>(currentMovementState)) Air->RequestAirborneMode(ModeClass);
}

void UStateMachineComponent::ClearAirborneMode()
{
    if (UAirContainerState* Air = Cast<UAirContainerState>(currentMovementState)) Air->ClearAirborneMode();
}

void UStateMachineComponent::RequestGroundedMode(TSubclassOf<UGroundedModeState> ModeClass)
{
    if (UGroundContainerState* Ground = Cast<UGroundContainerState>(currentMovementState)) Ground->RequestGroundedMode(ModeClass);
}

void UStateMachineComponent::ClearGroundedMode()
{
    if (UGroundContainerState* Ground = Cast<UGroundContainerState>(currentMovementState)) Ground->ClearGroundedMode();
}

bool UStateMachineComponent::IsAirborne() const { return HasActiveTag(airborneTag); }
bool UStateMachineComponent::IsGrounded() const { return HasActiveTag(groundedTag); }
/* ---------------- Unified Requests ---------------- */

void UStateMachineComponent::RequestAttack(const FVector2D& InputVector, EPlayerAction PlayerAction)
{
    if (currentActionState) currentActionState->OnAttackIntent(InputVector, PlayerAction);
}

void UStateMachineComponent::RequestBlockStart()
{
    if (currentActionState) currentActionState->OnBlockStartIntent();
}

void UStateMachineComponent::RequestBlockStop()
{
    if (currentActionState) currentActionState->OnBlockStopIntent();
}

void UStateMachineComponent::RequestDodge(const FVector2D &InputVector)
{
    if (currentActionState) currentActionState->OnDodgeIntent(InputVector);
}

void UStateMachineComponent::RequestJumpStart()
{
    const bool bConsumed = (currentActionState && currentActionState->OnJumpStartIntent());
    if (!bConsumed && currentMovementState) currentMovementState->OnJumpStartIntent();
}

void UStateMachineComponent::RequestJumpStop()
{
    const bool bConsumed = (currentActionState && currentActionState->OnJumpStopIntent());
    if (!bConsumed && currentMovementState) currentMovementState->OnJumpStopIntent();
}

void UStateMachineComponent::RequestLook(const FVector2D& InputVector)
{
    const bool bConsumed = (currentActionState && currentActionState->OnLookIntent(InputVector));
    if (!bConsumed && currentMovementState) currentMovementState->OnLookIntent(InputVector);
}

void UStateMachineComponent::RequestMove(const FVector2D& InputVector)
{
    const bool bConsumed = (currentActionState && currentActionState->OnMoveIntent(InputVector));
    if (!bConsumed && currentMovementState) currentMovementState->OnMoveIntent(InputVector);
}

void UStateMachineComponent::RequestMoveTo(AActor* Target, const FVector Loc, float AcceptanceRadius)
{
    const bool bConsumed = (currentActionState && currentActionState->OnMoveIntent(Target, Loc, AcceptanceRadius));
    if (!bConsumed && currentMovementState) currentMovementState->OnMoveIntent(Target, Loc, AcceptanceRadius);
}

/* ---------------- Character / Anim forwarding (unchanged) ---------------- */

void UStateMachineComponent::HandleJumpApexReached()
{
    if (currentActionState) currentActionState->OnJumpApexReached();
    if (currentMovementState) currentMovementState->OnJumpApexReached();
}

void UStateMachineComponent::HandleLanded(const FHitResult& Hit)
{
    if (currentActionState) currentActionState->OnLanded(Hit);

    // 1) Old state (air) reacts first
    if (currentMovementState) currentMovementState->OnLanded(Hit);

    // 2) Swap baseline
    ApplyBaselineMovement(false);

    // 3) Optional: let the new baseline (ground) react too
    if (currentMovementState) currentMovementState->OnLanded(Hit);
}

void UStateMachineComponent::HandleMovementModeChanged(ACharacter* InCharacter, EMovementMode PrevMovementMode, uint8 PrevCustomMode)
{
    // Let the state that was active during the mode change react first
    if (currentActionState) currentActionState->OnMovementModeChanged(InCharacter, PrevMovementMode, PrevCustomMode);
    if (currentMovementState) currentMovementState->OnMovementModeChanged(InCharacter, PrevMovementMode, PrevCustomMode);

    // Then swap baseline after handling
    ApplyBaselineMovement(false);
}

void UStateMachineComponent::OnAnimNotify(FGameplayTag NotifyTag)
{
    if (currentActionState)   currentActionState->OnAnimNotify(NotifyTag);
    if (currentMovementState) currentMovementState->OnAnimNotify(NotifyTag);
}

void UStateMachineComponent::OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
    if (currentActionState)   currentActionState->OnMontageBlendingOut(Montage, bInterrupted);
    if (currentMovementState) currentMovementState->OnMontageBlendingOut(Montage, bInterrupted);
}

/* -------------------- Combat Forwarding -----------------------*/

void UStateMachineComponent::OnReceiveHit(const FAtkHitData& HitData)
{
    UActionState* NewState = GetActionStateByTag(HitData.resolvedReaction);
    if (!NewState) return;

    ChangeActionState(NewState, false);

    if (currentActionState) currentActionState->ReceiveHit(HitData);
}