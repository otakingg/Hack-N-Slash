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

    //Create and initialize all state instances
    for (TPair<TSubclassOf<UCharacterState>, UCharacterState*>& Pair : stateInstances)
    {
        //Create instance if not already existing
        if (*Pair.Key && !Pair.Value) Pair.Value = NewObject<UCharacterState>(this, Pair.Key);

        //IF already existing or successfully created, initialize it
        if (Pair.Value) Pair.Value->Initialize(this, Cast<ACharacter>(GetOwner()));
    }
}

void UStateMachineComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

/************************************Private Functions************************************/
/************************************Private Functions************************************/

/************************************Protected Functions************************************/
/************************************Protected Functions************************************/

/************************************Public Functions************************************/
void UStateMachineComponent::ChangeState(UCharacterState *NewState, bool bForce)
{
    if (!NewState || NewState == currentState) return;

    // If NOT forced, ask state's for permission
    if (!bForce)
    {
        if (currentState && !currentState->CanExitState()) return;
        if (!NewState->CanEnterState(currentState)) return;
    }

    if (currentState) currentState->ExitState();

    previousState = currentState;
    currentState = NewState;

    currentState->EnterState();
}

UCharacterState* UStateMachineComponent::GetCurrentState() const {return currentState;}
UCharacterState *UStateMachineComponent::GetPreviousState() const { return previousState; }

FGameplayTag UStateMachineComponent::GetCurrentStateTag() const {return currentState ? currentState->GetStateTag() : FGameplayTag();}
//Allows "State.Combat.Attack.Light" and "State.Combat.Attack.Heavy" to match "State.Combat.Attack"
bool UStateMachineComponent::IsInStateTag(FGameplayTag Tag) const {return currentState && currentState->GetStateTag().MatchesTag(Tag);}
/************************************Public Functions************************************/