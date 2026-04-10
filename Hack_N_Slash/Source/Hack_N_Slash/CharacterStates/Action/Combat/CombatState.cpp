#include "CombatState.h"
#include "GameFramework/Character.h"

#include "../../../Tags/CharacterStateTagNamespaces.h"
#include "../../../Interfaces/CombatCmdInterface.h"
#include "../../../Interfaces/LocomotionCmdInterface.h"
#include "../../../Combat/Player/PlayerCombatCancelComponent.h"
#include "../../../Characters/Shared/StateMachineComponent.h"

bool UCombatState::OnAttackIntent(const FVector2D& InputVector)
{
    if (!ownerChar || (playerCombatCancelComp && !playerCombatCancelComp->CanCancel())) return false;
    
    ICombatCmdInterface* iCombatCmd = ownerStateMachineComp->GetCombatCommands();
    if (!iCombatCmd) return false;

    if (playerCombatCancelComp) // If this is the player, cancel current action and attack
    {
        iCombatCmd->AttackIntent(InputVector);
        return true;
    }
    else return false; // AI will always be set to none state before they attack, so they can't attack in this state
}

bool UCombatState::OnJumpStartIntent()
{
    Super::OnJumpStartIntent();

    if (!ownerChar || (playerCombatCancelComp && !playerCombatCancelComp->CanCancel())) return false;
    
    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd()) locoCMD->JumpStart();
    else ownerChar->Jump();
    return true;
}

bool UCombatState::OnJumpStopIntent()
{
    if (!ownerChar) return false;

    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd()) locoCMD->JumpStop();
    else ownerChar->StopJumping();
    return true;
}