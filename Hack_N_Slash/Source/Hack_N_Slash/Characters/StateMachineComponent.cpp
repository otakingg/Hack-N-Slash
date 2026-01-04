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

    ownerChar = Cast<ACharacter>(GetOwner());
    InitializeMovementMap();
    InitializeActionMap();

    //*StateClass extracts the raw UClass* from the TSubclassOf. In this case RootMovementState*
    if (!currentMovementState && *defaultMovementStateClass)
    {
        if (UMovementState* Found = GetMovementState(defaultMovementStateClass)) ChangeMovementState(Found, true);
    }

    if (!currentActionState && *defaultActionStateClass)
    {
        if (UActionState* Found = GetActionState(defaultActionStateClass)) ChangeActionState(Found, true);
    }
}

/* ---------------- Initialization ---------------- */

void UStateMachineComponent::InitializeMovementMap()
{
    movementStateInstances.Empty();

    for (const TSubclassOf<UMovementState>& StateClass : movementStateClasses)
    {
        if (!*StateClass) continue;

        UMovementState* Instance = NewObject<UMovementState>(this, StateClass);
        if (!Instance) continue;

        Instance->Initialize(this, ownerChar);
        movementStateInstances.Add(StateClass, Instance);
    }
}

void UStateMachineComponent::InitializeActionMap()
{
    actionStateInstances.Empty();

    for (const TSubclassOf<UActionState>& StateClass : actionStateClasses)
    {
        if (!*StateClass) continue;

        UActionState* Instance = NewObject<UActionState>(this, StateClass);
        if (!Instance) continue;

        Instance->Initialize(this, ownerChar);
        actionStateInstances.Add(StateClass, Instance);
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

UActionState* UStateMachineComponent::GetActionState(TSubclassOf<UActionState> StateClass)
{
    if (const TObjectPtr<UActionState>* Found = actionStateInstances.Find(StateClass)) return Found->Get();
    return nullptr;
}

UMovementState* UStateMachineComponent::GetMovementState(TSubclassOf<UMovementState> StateClass)
{
    if (const TObjectPtr<UMovementState>* Found = movementStateInstances.Find(StateClass)) return Found->Get();
    return nullptr;
}

bool UStateMachineComponent::IsInMovementTag(FGameplayTag Tag) const { return currentMovementState && currentMovementState->GetStateTag().MatchesTag(Tag); }
bool UStateMachineComponent::IsInActionTag(FGameplayTag Tag) const {return currentActionState && currentActionState->GetStateTag().MatchesTag(Tag);}
bool UStateMachineComponent::IsInAnyTag(FGameplayTag Tag) const
{
    // Action layer overrides movement
    if (IsInActionTag(Tag)) return true;
    return IsInMovementTag(Tag);
}

/* ---------------- Event Forwarding ---------------- */
void UStateMachineComponent::OnInputAttackPressed(const FVector2D& InputVector)
{
    if (currentActionState) currentActionState->OnInputAttackPressed(InputVector);
}

void UStateMachineComponent::OnInputBlockDodgePressed(const FVector2D& InputVector)
{
    if (currentActionState) currentActionState->OnInputBlockDodgePressed(InputVector);
}

void UStateMachineComponent::OnInputJumpPressed()
{
    const bool bConsumed = (currentActionState && currentActionState->OnInputJumpPressed());
    if (!bConsumed && currentMovementState) currentMovementState->OnInputJumpPressed();
}

void UStateMachineComponent::OnInputJumpReleased()
{
    const bool bConsumed = (currentActionState && currentActionState->OnInputJumpReleased());
    if (!bConsumed && currentMovementState) currentMovementState->OnInputJumpReleased();
}

void UStateMachineComponent::OnInputLook(const FVector2D& InputVector)
{
    const bool bConsumed = (currentActionState && currentActionState->OnInputLook(InputVector));
    if (!bConsumed && currentMovementState) currentMovementState->OnInputLook(InputVector);
}

void UStateMachineComponent::OnInputMove(const FVector2D& InputVector)
{
    const bool bConsumed = (currentActionState && currentActionState->OnInputMove(InputVector));
    if (!bConsumed && currentMovementState) currentMovementState->OnInputMove(InputVector);
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