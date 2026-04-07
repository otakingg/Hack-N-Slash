#include "PlayerCombatCancelComponent.h"
#include "../../Tags/CharacterStateTagNamespaces.h"
#include "../../Characters/StateMachineComponent.h"

UPlayerCombatCancelComponent::UPlayerCombatCancelComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerCombatCancelComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPlayerCombatCancelComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UPlayerCombatCancelComponent::CanCancel(EPlayerAction Action, UStateMachineComponent* StateMachineComp) const
{
	if (!StateMachineComp) return false;
	else if (StateMachineComp->HasExactActiveTag(ActionTags::None) || StateMachineComp->HasExactActiveTag(CombatTags::Block)) return true; //If character is in no action state, there's nothing to cancel. Block can be canceled into anything
	else if (!bCanCancelCurrentAction) return false;

	switch(Action)
	{
		case EPlayerAction::AttackHeavyStart:
		case EPlayerAction::AttackLightStart:
		case EPlayerAction::BlockStart:
		case EPlayerAction::BlockStop:
		case EPlayerAction::Dodge:
		case EPlayerAction::JumpStart:
		case EPlayerAction::JumpStop:
		default: return true;
	}
}