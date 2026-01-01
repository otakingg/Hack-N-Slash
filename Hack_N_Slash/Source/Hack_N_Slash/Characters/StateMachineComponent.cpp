#include "StateMachineComponent.h"
#include "GameFramework/Character.h"

UStateMachineComponent::UStateMachineComponent()
{
    // Fully event-driven
    PrimaryComponentTick.bCanEverTick = false;
}

void UStateMachineComponent::BeginPlay()
{
    Super::BeginPlay();

    InitializeMovementMap();
    InitializeActionMap();

    // Enter defaults (optional, but recommended)
    if (!currentMovementState && *defaultMovementStateClass)
    {
        if (UMovementState** Found = movementStateInstances.Find(defaultMovementStateClass)) ChangeMovementState(*Found, true);
    }

    if (!currentActionState && *defaultActionStateClass)
    {
        if (UActionState** Found = actionStateInstances.Find(defaultActionStateClass)) ChangeActionState(*Found, true);
    }
}

/* ---------------- Initialization ---------------- */

void UStateMachineComponent::InitializeMovementMap()
{
    ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());

    for (TPair<TSubclassOf<UMovementState>, UMovementState*>& Pair : movementStateInstances)
    {
        if (*Pair.Key && !Pair.Value) Pair.Value = NewObject<UMovementState>(this, Pair.Key);

        if (Pair.Value) Pair.Value->Initialize(this, OwnerChar);
    }
}

void UStateMachineComponent::InitializeActionMap()
{
    ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());

    for (TPair<TSubclassOf<UActionState>, UActionState*>& Pair : actionStateInstances)
    {
        if (*Pair.Key && !Pair.Value) Pair.Value = NewObject<UActionState>(this, Pair.Key);

        if (Pair.Value) Pair.Value->Initialize(this, OwnerChar);
    }
}

/* ---------------- Transition Rules ---------------- */

bool UStateMachineComponent::CanTransition(const UCharacterState* Current, const UCharacterState* Next, bool bForce)
{
    if (!Next || Next == Current) return false;
    if (bForce) return true;

    if (Current && !Current->CanExitState()) return false;
    if (!Next->CanEnterState(Current)) return false;

    // Priority / interruption rule (mainly for Action, harmless for Movement)
    if (Current && !Current->CanBeInterruptedBy(Next)) return false;

    return true;
}

/* ---------------- State Changes ---------------- */

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

void UStateMachineComponent::ChangeState(EStateLayer Layer, UCharacterState* NewState, bool bForce)
{
    if (Layer == EStateLayer::Movement) ChangeMovementState(Cast<UMovementState>(NewState), bForce);
    else ChangeActionState(Cast<UActionState>(NewState), bForce);
}

/* ---------------- Tag Queries ---------------- */

bool UStateMachineComponent::IsInMovementTag(FGameplayTag Tag) const {return currentMovementState && currentMovementState->GetStateTag().MatchesTag(Tag);}
bool UStateMachineComponent::IsInActionTag(FGameplayTag Tag) const {return currentActionState && currentActionState->GetStateTag().MatchesTag(Tag);}
bool UStateMachineComponent::IsInAnyTag(FGameplayTag Tag) const
{
    // Action layer overrides movement
    if (IsInActionTag(Tag)) return true;
    return IsInMovementTag(Tag);
}

/* ---------------- Event Forwarding ---------------- */
void UStateMachineComponent::OnInputAttackPressed()
{
    if (currentActionState) currentActionState->OnInputAttackPressed();
}

void UStateMachineComponent::OnInputBlockDodgePressed()
{
    if (currentActionState) currentActionState->OnInputBlockDodgePressed();
}

void UStateMachineComponent::OnInputJumpPressed()
{
    if (currentActionState)   currentActionState->OnInputJumpPressed();
    if (currentMovementState) currentMovementState->OnInputJumpPressed();
}

void UStateMachineComponent::OnInputJumpReleased()
{
    if (currentActionState)   currentActionState->OnInputJumpReleased();
    if (currentMovementState) currentMovementState->OnInputJumpReleased();
}

void UStateMachineComponent::OnInputMoveStarted()
{
    if (currentActionState)   currentActionState->OnInputMoveStarted();
    if (currentMovementState) currentMovementState->OnInputMoveStarted();
}

void UStateMachineComponent::OnInputMoveStopped()
{
    if (currentActionState)   currentActionState->OnInputMoveStopped();
    if (currentMovementState) currentMovementState->OnInputMoveStopped();
}

void UStateMachineComponent::OnLanded(const FHitResult& Hit)
{
    // Action first (e.g., Knockdown reacts to landing)
    if (currentActionState) currentActionState->OnLanded(Hit);
    if (currentMovementState) currentMovementState->OnLanded(Hit);
}

void UStateMachineComponent::OnMovementModeChanged(EMovementMode PrevMode, uint8 PrevCustomMode)
{
    if (currentActionState) currentActionState->OnMovementModeChanged(PrevMode, PrevCustomMode);
    if (currentMovementState) currentMovementState->OnMovementModeChanged(PrevMode, PrevCustomMode);
}

void UStateMachineComponent::OnAnimNotify(FName NotifyName)
{
    // Some movement states may care (TurnInPlace), but action usually cares more.
    if (currentActionState)   currentActionState->OnAnimNotify(NotifyName);
    if (currentMovementState) currentMovementState->OnAnimNotify(NotifyName);
}

void UStateMachineComponent::OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
    if (currentActionState)   currentActionState->OnMontageBlendingOut(Montage, bInterrupted);
    if (currentMovementState) currentMovementState->OnMontageBlendingOut(Montage, bInterrupted);
}