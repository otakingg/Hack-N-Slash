#include "AttackState.h"
#include "GameFramework/Character.h"
#include "../../../Interfaces/CombatCmdInterface.h"
#include "../../../Combat/Player/PlayerCombatCancelComponent.h"
#include "../../../Characters/Shared/StateMachineComponent.h"

bool UAttackState::OnAttackIntent(const FVector2D& InputVector)
{
    ICombatCmdInterface* iCombatCmd = ownerStateMachineComp->GetCombatCommands();
    if (!iCombatCmd) return false;

    if (playerCombatCancelComp) // If this is the player, check and see if we can cancel
    {
        if (playerCombatCancelComp->CanCancel()) // If can cancel, then attack and consume input
        {
            iCombatCmd->AttackIntent(InputVector);
            return true;
        }
        else return false;
    }
    else return false; // AI will always be set to none state before they attack, so they don't need to check for cancel
}