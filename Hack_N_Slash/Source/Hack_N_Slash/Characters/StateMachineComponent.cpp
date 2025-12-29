// Fill out your copyright notice in the Description page of Project Settings.
#include "StateMachineComponent.h"
#include "Gameframework/Character.h"

UStateMachineComponent::UStateMachineComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

void UStateMachineComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UStateMachineComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (currentState) currentState->TickState(DeltaTime);
}

/************************************Private Functions************************************/
void UStateMachineComponent::ChangeStateInternal(UCharacterState *NewState, bool bForce)
{
    if (currentState) currentState->ExitState();

    currentState = NewState;

    if (currentState) currentState->EnterState();
}
/************************************Private Functions************************************/

/************************************Protected Functions************************************/
/************************************Protected Functions************************************/

/************************************Public Functions************************************/
bool UStateMachineComponent::TryChangeState(TSubclassOf<UCharacterState> NewStateClass)
{
    if (!NewStateClass) return false;

    UCharacterState* NewState = stateInstances.FindRef(NewStateClass);
    if (!NewState)
    {
        NewState = NewObject<UCharacterState>(this, NewStateClass);
        NewState->Initialize(this, Cast<ACharacter>(GetOwner()));
        stateInstances.Add(NewStateClass, NewState);
    }

    if (currentState)
    {
        if (!currentState->CanExitState()) return false;

        if (!currentState->CanBeInterruptedBy(NewState)) return false;
    }

    if (!NewState->CanEnterState()) return false;

    ChangeStateInternal(NewState, false);
    return true;
}

void UStateMachineComponent::ForceChangeState(TSubclassOf<UCharacterState> NewStateClass)
{
    if (!NewStateClass) return;

    UCharacterState* NewState = stateInstances.FindRef(NewStateClass);
    if (!NewState)
    {
        NewState = NewObject<UCharacterState>(this, NewStateClass);
        NewState->Initialize(this, Cast<ACharacter>(GetOwner()));
        stateInstances.Add(NewStateClass, NewState);
    }

    ChangeStateInternal(NewState, true);
}

UCharacterState *UStateMachineComponent::GetCurrentState() const {return currentState;}

bool UStateMachineComponent::IsInStateTag(FGameplayTag Tag) const {return currentState && currentState->GetStateTag() == Tag;}
/************************************Public Functions************************************/