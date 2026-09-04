#include "StateMachineComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../../Structs/FAtkHitData.h"

UStateMachineComponent::UStateMachineComponent() { PrimaryComponentTick.bCanEverTick = false; }

void UStateMachineComponent::BeginPlay()
{
    Super::BeginPlay();
    
    ownerChar = Cast<ACharacter>(GetOwner());

    InitializeMovementMap();
    InitializeActionMap();

    if (ownerChar)
    {
        ownerChar->LandedDelegate.AddDynamic(this, &UStateMachineComponent::HandleLanded);
        ownerChar->MovementModeChangedDelegate.AddDynamic(this, &UStateMachineComponent::HandleMovementModeChanged);
        ownerChar->OnReachedJumpApex.AddDynamic(this, &UStateMachineComponent::HandleJumpApexReached);
    }

    DecideMovementState(true);

    UActionState* noneState = GetActionStateByTag(Tags::StateMachine::Action::None);
    if (!noneState && bDebug) UE_LOG(LogTemp, Warning, TEXT("[%s] Default Action State not registered: %s"), *GetNameSafe(this), *Tags::StateMachine::Action::None.ToString());
    ChangeActionState(noneState, true);
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
void UStateMachineComponent::InitializeActionMap()
{
    for (auto& state : actionStateInstances) delete state;
    actionStateInstances.Empty();

    for (const TSubclassOf<UActionState>& stateClass : actionStateClasses)
    {
        UClass* classKey = stateClass.Get();
        if (!classKey || classKey->HasAnyClassFlags(EClassFlags::CLASS_Abstract)) continue;

        UActionState* instance = NewObject<UActionState>(this, classKey);
        if (!instance) continue;

        instance->Initialize(this, ownerChar);
        actionStateInstances.Add(instance);
    }
}

void UStateMachineComponent::InitializeMovementMap()
{
    for (auto& state : movementStateInstances) delete state;
    movementStateInstances.Empty();

    for (const TSubclassOf<UMovementState>& stateClass : movementStateClasses)
    {
        UClass* classKey = stateClass.Get();
        if (!classKey || classKey->HasAnyClassFlags(EClassFlags::CLASS_Abstract)) continue;

        UMovementState* instance = NewObject<UMovementState>(this, classKey);
        if (!instance) continue;

        instance->Initialize(this, ownerChar);
        movementStateInstances.Add(instance);
    }
}

// ---------------- State Lookup ----------------
UActionState* UStateMachineComponent::GetActionStateByTag(const FGameplayTag& Tag) const
{
    if (!Tag.IsValid()) return nullptr;

    for (const auto& state : actionStateInstances) if (state && state->GetStateTag().MatchesTagExact(Tag)) return state;

    return nullptr;
}

UMovementState* UStateMachineComponent::GetMovementStateByTag(const FGameplayTag& Tag) const
{
    if (!Tag.IsValid()) return nullptr;
    
    for (const auto& state : movementStateInstances) if (state && state->GetStateTag().MatchesTagExact(Tag)) return state;

    return nullptr;
}

/* ---------------- Transition Rules ---------------- */

bool UStateMachineComponent::CanTransition(const UCharacterState* Current, const UCharacterState* Next, bool bForce)
{
    if (!Next) return false;

    if (bForce) return true;

    if ((Current && !Current->CanExitState()) || !Next->CanEnterState(Current)) return false;

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

    return true;
}

void UStateMachineComponent::ClearActionState()
{
    UActionState* noneState = GetActionStateByTag(Tags::StateMachine::Action::None);
    ChangeActionState(noneState, true);
}

void UStateMachineComponent::DecideMovementState(bool bForce) { for (const auto& state : movementStateInstances) if (ChangeMovementState(state, bForce)) break; }

/* ---------------- Event Forwarding ---------------- */

// Movement Events
void UStateMachineComponent::HandleJumpApexReached() { if (currentActionState) currentActionState->OnJumpApexReached(); }

void UStateMachineComponent::HandleLanded(const FHitResult& Hit)
{
    DecideMovementState(false); // Refresh movement state when landing
    if (currentActionState) currentActionState->OnLanded(Hit); // Let the current action state react to landing
}

void UStateMachineComponent::HandleMovementModeChanged(ACharacter* InCharacter, EMovementMode PrevMovementMode, uint8 PrevCustomMode)
{
    DecideMovementState(false); // Refresh movement state when movement mode is changed
    if (currentActionState) currentActionState->OnMovementModeChanged(InCharacter, PrevMovementMode, PrevCustomMode); // Let the current action state react to movement mode changed
}

// Anim Events
void UStateMachineComponent::HandleAnimNotify(FGameplayTag NotifyTag) { if (currentActionState) currentActionState->OnAnimNotify(NotifyTag); }

// Combat Events
void UStateMachineComponent::HandleReceiveHit(const FAtkHitData& HitData)
{
    if (HitData.resolvedReaction == Tags::StateMachine::Action::None || HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::NoReact) return;

    // Get the reaction state by tag
    UActionState* reactionState = nullptr;
    if (HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::Dead) reactionState = GetActionStateByTag(Tags::StateMachine::Action::Reaction::Dead);
    else if (HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::BlockHit) reactionState = GetActionStateByTag(Tags::StateMachine::Action::Combat::Block);
    else if (HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::BlockPerfect) reactionState = GetActionStateByTag(Tags::StateMachine::Action::Combat::Block);
    else reactionState = GetActionStateByTag(Tags::StateMachine::Action::Reaction::Hit);

    if (!reactionState) return;

    ChangeActionState(reactionState, false);
    if (currentActionState) currentActionState->ReceiveHit(HitData); // Let the chosen reaction state handle hit received
}

void UStateMachineComponent::HandleCountered(AActor* Counteror, const FString& Reason)
{
    UActionState* hitState = GetActionStateByTag(Tags::StateMachine::Action::Reaction::Hit);
    if (!hitState) return;

    ChangeActionState(hitState, false);

    if (currentActionState)
    {
        FAtkHitData hitData = FAtkHitData();
        hitData.attacker = Counteror;
        hitData.damager = Counteror;
        hitData.resolvedReaction = Tags::StateMachine::Action::Reaction::Countered;

        currentActionState->ReceiveHit(hitData);
    }
}

/* --------------------- Intent Hoooking ----------------- */
FGameplayTag UStateMachineComponent::ResolvePlayerInput(EPlayerInput PlayerInput, const FVector2D& LookVector, const FVector2D& MoveVector)
{
    if (PlayerInput == EPlayerInput::None || !currentMovementState || !currentActionState) return Tags::PlayerAction::None;

    // MUST BE DONE IN THIS ORDER!!!
    FGameplayTag potentialAction = currentMovementState->ResolvePlayerInput(PlayerInput, LookVector, MoveVector);
    return currentActionState->ResolvePlayerAction(potentialAction);
}