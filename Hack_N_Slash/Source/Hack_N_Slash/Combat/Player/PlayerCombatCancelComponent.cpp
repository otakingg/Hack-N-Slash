#include "PlayerCombatCancelComponent.h"
#include "../../Tags/CharacterStateTagNamespaces.h"
#include "../../Combat/Player/PlayerCombatComponent.h"
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

bool UPlayerCombatCancelComponent::CanCancel(UStateMachineComponent* StateMachineComp, UPlayerCombatComponent* CombatComp) const
{
	if (!StateMachineComp) return false;
	else if (StateMachineComp->HasExactActiveTag(ActionTags::None) || StateMachineComp->HasExactActiveTag(CombatTags::Block)) return true; //If character is in no action state, there's nothing to cancel. Block can be canceled into anything
	else if (!bCanCancelCurrentAction) return false;

	FGameplayTagContainer cancelableStates; // States that the current action can be cancelled into
	if (StateMachineComp->IsInActionTag(CombatTags::Attack))
	{
		if (!CombatComp) return true;
		FAtkData* currentAtkData = CombatComp->GetCurrentAtkData();
		if (!currentAtkData) return true;
		else
		{
			currentAtkData->FillCancelableCombatStateContainer();
			cancelableStates = currentAtkData->cancelableCombatStateContainer;
		}
	}
	else if (StateMachineComp->IsInActionTag(CombatTags::Dodge))
	{
		for (const FGameplayTag& tag : cancelableDodgeStates) cancelableStates.AddTag(tag);
	}
	else if (StateMachineComp->IsInActionTag(CombatTags::Jump))
	{
		for (const FGameplayTag& tag : cancelableJumpStates) cancelableStates.AddTag(tag);
	}
	else return true; //If character is in an action state that isn't attack, dodge, or jump, allow canceling (This allows for new actions to be added without needing to update this function, but it also means that if we do add a new action state, we need to make sure to add the appropriate tags to this function)

	if (cancelableStates.IsEmpty()) return false;
	return StateMachineComp->GetActiveStateTags().HasAnyExact(cancelableStates);
}

void UPlayerCombatCancelComponent::SetCanCancelCurrentAction(bool bCanCancel) { bCanCancelCurrentAction = bCanCancel; }