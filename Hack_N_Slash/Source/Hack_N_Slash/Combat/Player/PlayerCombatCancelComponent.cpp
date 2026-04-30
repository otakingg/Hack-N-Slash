#include "PlayerCombatCancelComponent.h"
#include "../../Tags/CharacterStateTagNamespaces.h"
#include "../../Combat/Player/PlayerCombatComponent.h"
#include "../../Characters/Shared/StateMachineComponent.h"

UPlayerCombatCancelComponent::UPlayerCombatCancelComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerCombatCancelComponent::BeginPlay()
{
	Super::BeginPlay();

    owner = GetOwner();
    if (!owner) return;

	combatComp = owner->FindComponentByClass<UPlayerCombatComponent>();
	stateMachineComp = owner->FindComponentByClass<UStateMachineComponent>();
}

void UPlayerCombatCancelComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UPlayerCombatCancelComponent::CanCancel(FGameplayTag& DesiredStateTag) const
{
	if (!stateMachineComp) return true;
	else if (stateMachineComp->HasExactActiveTag(CombatTags::Block) || stateMachineComp->HasExactActiveTag(CombatTags::Jump)) return true; // Block and Jump can be canceled into anything
	else if (!bCanCancelCurrentAction) return false;

	FGameplayTagContainer cancelableStates; // States that the current action can be cancelled into
	if (stateMachineComp->IsInExactActionTag(CombatTags::Attack))
	{
		if (!combatComp) return true;
		FPlayerAtkData* currentAtkData = combatComp->GetCurrentAtkData();
		if (!currentAtkData) return true;
		else
		{
			if (currentAtkData->cancelableCombatStateContainer.IsEmpty()) currentAtkData->FillCancelableCombatStateContainer();
			cancelableStates = currentAtkData->cancelableCombatStateContainer;
		}
	}
	else if (stateMachineComp->IsInExactActionTag(CombatTags::Dodge))
	{
		for (const FGameplayTag& tag : cancelableDodgeStates) cancelableStates.AddTag(tag);
	}
	/*else if (stateMachineComp->IsInExactActionTag(CombatTags::Jump))
	{
		for (const FGameplayTag& tag : cancelableJumpStates) cancelableStates.AddTag(tag);
	}*/
	else return true; //If character is in an action state that isn't attack, dodge, or jump, allow canceling (This allows for new actions to be added without needing to update this function, but it also means that if we do add a new action state, we need to make sure to add the appropriate tags to this function)

	if (cancelableStates.IsEmpty()) return false;
	else return cancelableStates.HasTagExact(DesiredStateTag);
}

void UPlayerCombatCancelComponent::SetCanCancelCurrentAction(bool bCanCancel) { bCanCancelCurrentAction = bCanCancel; }