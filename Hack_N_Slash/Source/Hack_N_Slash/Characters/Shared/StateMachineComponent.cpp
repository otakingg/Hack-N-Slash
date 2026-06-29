#include "StateMachineComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../../Tags/CharacterStateTags.h"
#include "../../Interfaces/Damageable.h"
#include "../../Structs/FAtkHitData.h"
#include "LocomotionComponent.h"

UStateMachineComponent::UStateMachineComponent() { PrimaryComponentTick.bCanEverTick = false; }

void UStateMachineComponent::BeginPlay()
{
    Super::BeginPlay();
    
    ownerChar = Cast<ACharacter>(GetOwner());
    locoComp = ownerChar ? ownerChar->FindComponentByClass<ULocomotionComponent>() : nullptr;

    InitializeMovementMap();
    InitializeActionMap();

    if (ownerChar)
    {
        ownerChar->LandedDelegate.AddDynamic(this, &UStateMachineComponent::HandleLanded);
        ownerChar->MovementModeChangedDelegate.AddDynamic(this, &UStateMachineComponent::HandleMovementModeChanged);
        ownerChar->OnReachedJumpApex.AddDynamic(this, &UStateMachineComponent::HandleJumpApexReached);
    }

    DecideMovementState(true);

    UActionState* desiredState = GetActionStateByTag(defaultActionTag);
    if (!desiredState && bDebug) UE_LOG(LogTemp, Warning, TEXT("[%s] Default Action State not registered: %s"), *GetNameSafe(this), *defaultActionTag.ToString());
    ChangeActionState(desiredState, true);

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

// ---------------- Initialization ----------------
void UStateMachineComponent::InitializeMovementMap()
{
    movementStateInstances.Empty();

    for (const TSubclassOf<UMovementState>& StateClass : movementStateClasses)
    {
        UClass* ClassKey = StateClass.Get();
        if (!ClassKey || ClassKey->HasAnyClassFlags(EClassFlags::CLASS_Abstract)) continue;

        if (movementStateInstances.Contains(ClassKey)) continue; // Prevent duplicates

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

// ---------------- State Lookup ----------------
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

    if (currentMovementState) activeStateTags.AddTag(currentMovementState->GetStateTag());
    if (currentActionState) activeStateTags.AddTag(currentActionState->GetStateTag());
}

bool UStateMachineComponent::HasActiveTag(const FGameplayTag& Tag) const { return activeStateTags.HasTag(Tag); }
bool UStateMachineComponent::HasExactActiveTag(const FGameplayTag& Tag) const { return activeStateTags.HasTagExact(Tag); }

bool UStateMachineComponent::IsAirborne() const { return HasActiveTag(airborneTag); }
bool UStateMachineComponent::IsGrounded() const { return HasActiveTag(groundedTag); }
/* ---------------- Transition Rules ---------------- */

bool UStateMachineComponent::CanTransition(const UCharacterState* Current, const UCharacterState* Next, bool bForce)
{
    if (!Next) return false;
    if (bForce) return true;

    if (Current && !Current->CanExitState()) return false;
    if (!Next->CanEnterState(Current)) return false;

    return true;
}

/* ---------------- State changes ---------------- */
bool UStateMachineComponent::ChangeMovementState(UMovementState* NewState, bool bForce)
{
    if (!CanTransition(currentMovementState, NewState, bForce)) return false;

    if (currentMovementState == NewState) return true;

    if (currentMovementState) currentMovementState->ExitState();
    previousMovementState = currentMovementState;
    currentMovementState = NewState;
    currentMovementState->EnterState();

    RebuildActiveStateTags();
    return true;
}

bool UStateMachineComponent::ChangeActionState(UActionState* NewState, bool bForce)
{
    if (!CanTransition(currentActionState, NewState, bForce)) return false;

    if (currentActionState == NewState) return true;

    if (currentActionState) currentActionState->ExitState();
    previousActionState = currentActionState;
    currentActionState = NewState;
    currentActionState->EnterState();

    RebuildActiveStateTags();
    return true;
}

void UStateMachineComponent::ClearActionState()
{
    UActionState* noneState = GetActionStateByTag(StateActionTags::None);
    ChangeActionState(noneState, true);
}

void UStateMachineComponent::DecideMovementState(bool bForce) { for (auto& pair : movementStateInstances) if (ChangeMovementState(pair.Value, bForce)) break; }

/* ---------------- Event Forwarding ---------------- */

// Movement Events
void UStateMachineComponent::HandleJumpApexReached()
{
    if (currentMovementState) currentMovementState->OnJumpApexReached();
    if (currentActionState) currentActionState->OnJumpApexReached();
}

void UStateMachineComponent::HandleLanded(const FHitResult& Hit)
{
    // 1) Old state (air) reacts first
    if (currentMovementState) currentMovementState->OnLanded(Hit);

    // 2) Swap baseline
    DecideMovementState(false);

    // 3) Let the new baseline react too
    if (currentMovementState) currentMovementState->OnLanded(Hit);

    if (currentActionState) currentActionState->OnLanded(Hit);
}

void UStateMachineComponent::HandleMovementModeChanged(ACharacter* InCharacter, EMovementMode PrevMovementMode, uint8 PrevCustomMode)
{
    if (currentMovementState) currentMovementState->OnMovementModeChanged(InCharacter, PrevMovementMode, PrevCustomMode);
    if (currentActionState) currentActionState->OnMovementModeChanged(InCharacter, PrevMovementMode, PrevCustomMode);
    DecideMovementState(false);
    if (locoComp) locoComp->ApplyMovementFromTagsAndStats();
}

// Anim Events
void UStateMachineComponent::HandleAnimNotify(FGameplayTag NotifyTag) { if (currentActionState) currentActionState->OnAnimNotify(NotifyTag); }

// Combat Events
void UStateMachineComponent::HandleReceiveHit(const FAtkHitData& HitData)
{
    UActionState* reactionState = nullptr;

    if (IDamageable* iDmgble = Cast<IDamageable>(ownerChar))
    {
        if (!iDmgble->IsAlive()) reactionState = GetActionStateByTag(StateReactionTags::Dead);
        else reactionState = GetActionStateByTag(StateReactionTags::Hit);
    }
    else if (HitData.resolvedReaction == StateReactionTags::BlockHit) reactionState = GetActionStateByTag(StateCombatTags::Block);
    else reactionState = GetActionStateByTag(StateReactionTags::Hit);

    if (!reactionState) return;

    ChangeActionState(reactionState, false);
    if (currentActionState) currentActionState->ReceiveHit(HitData);
}

void UStateMachineComponent::HandleCountered(AActor* Counteror, const FString& Reason)
{
    UActionState* hitState = GetActionStateByTag(StateReactionTags::Hit);
    if (!hitState) return;

    ChangeActionState(hitState, false);

    if (currentActionState)
    {
        FAtkHitData hitData = FAtkHitData();
        hitData.attacker = Counteror;
        hitData.damager = Counteror;
        hitData.resolvedReaction = StateReactionTags::Countered;

        currentActionState->ReceiveHit(hitData);
    }
}

/* --------------------- Intent Hoooking ----------------- */
FGameplayTag UStateMachineComponent::ResolvePlayerInput(EPlayerInput PlayerInput, const FVector2D& InputVector)
{
    if (PlayerInput == EPlayerInput::None || !currentMovementState || !currentActionState) return CharacterActionTags::None;

    // MUST BE DONE IN THIS ORDER!!!
    FGameplayTag potentialAction = currentMovementState->ResolvePlayerInput(PlayerInput, InputVector);
    return currentActionState->ResolvePlayerAction(potentialAction, InputVector);
}