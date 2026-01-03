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

    // Enter defaults (optional, but recommended)
    if (!currentMovementState && *defaultMovementStateClass)
    {
        //.Find returns a pointer to the value, in this case UMovementState*
        //Because .Find returns a pointer, and the value in this case is also a pointer, the returnt type has 2 asterisks
        //*Found dereferences the first pointer to get the actual UMovementState* pointer
        if (UMovementState** Found = movementStateInstances.Find(defaultMovementStateClass)) ChangeMovementState(*Found, true);
    }

    //*defaultActionStateClass = “Give me the underlying UClass* stored inside this TSubclassOf"
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

UActionState* UStateMachineComponent::GetActionState(TSubclassOf<UActionState> StateClass)
{
    if (!*StateClass) return nullptr;

    UActionState** state = actionStateInstances.Find(StateClass);
    if (!state) //If state class not found in map
    {
        if (bDebug && GEngine)
        {
            FString Msg = FString::Printf(TEXT("Action state not registered: %s"), *GetNameSafe(*StateClass));
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, Msg);
        }
        ensureMsgf(false, TEXT("Action state not registered: %s"), *GetNameSafe(*StateClass));
        return nullptr;
    }

    //If the state instance pointer is null, create the instance now
    if (!*state) *state = NewObject<UActionState>(this, StateClass);
    return *state;
}

UMovementState* UStateMachineComponent::GetMovementState(TSubclassOf<UMovementState> StateClass)
{
    if (!*StateClass) return nullptr;

    UMovementState** state = movementStateInstances.Find(StateClass);
    if (!state) //If state class not found in map
    {
        if (bDebug && GEngine)
        {
            FString Msg = FString::Printf(TEXT("Movement state not registered: %s"), *GetNameSafe(*StateClass));
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, Msg);
        }
        ensureMsgf(false, TEXT("Movement state not registered: %s"), *GetNameSafe(*StateClass));
        return nullptr;
    }

    //If the state instance pointer is null, create the instance now
    if (!*state) *state = NewObject<UMovementState>(this, StateClass);
    return *state;
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
    if (currentActionState)   currentActionState->OnInputJumpPressed();
    if (currentMovementState) currentMovementState->OnInputJumpPressed();
}

void UStateMachineComponent::OnInputJumpReleased()
{
    if (currentActionState)   currentActionState->OnInputJumpReleased();
    if (currentMovementState) currentMovementState->OnInputJumpReleased();
}

void UStateMachineComponent::OnInputLook(const FVector2D& InputVector)
{
    if (currentActionState)   currentActionState->OnInputLook(InputVector);
    if (currentMovementState) currentMovementState->OnInputLook(InputVector);
}

void UStateMachineComponent::OnInputMove(const FVector2D& InputVector)
{
    if (currentActionState)   currentActionState->OnInputMove(InputVector);
    if (currentMovementState) currentMovementState->OnInputMove(InputVector);
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