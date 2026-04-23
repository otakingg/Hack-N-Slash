#include "CombatState.h"
#include "GameFramework/Character.h"

#include "../../../Tags/CharacterStateTagNamespaces.h"
#include "../../../Interfaces/CombatCmdInterface.h"
#include "../../../Interfaces/LocomotionCmdInterface.h"
#include "../../../Tags/LocomotionTags.h"
#include "../../../Combat/Player/PlayerCombatCancelComponent.h"
#include "../../../Characters/Shared/StateMachineComponent.h"

void UCombatState::EnterState()
{
    Super::EnterState();
    if (ILocomotionCmdInterface* iLocoCmd = GetLocoCmd()) iLocoCmd->AddMoveOverrideTag(TAG_Move_Override_Lock);
}

void UCombatState::ExitState()
{
    if (ILocomotionCmdInterface* iLocoCmd = GetLocoCmd()) iLocoCmd->RemoveMoveOverrideTag(TAG_Move_Override_Lock);
    Super::ExitState();
}

bool UCombatState::OnAttackIntent(const FVector2D &InputVector, EPlayerAction PlayerAction)
{
    if (!ownerChar || (playerCombatCancelComp && !playerCombatCancelComp->CanCancel())) return false;
    
    ICombatCmdInterface* iCombatCmd = ownerStateMachineComp->GetCombatCommands();
    if (!iCombatCmd) return false;

    if (playerCombatCancelComp) // If this is the player, cancel current action and attack
    {
        iCombatCmd->AttackIntent(InputVector, PlayerAction);
        return true;
    }
    else return false; // AI will always be set to none state before they attack, so they can't attack in this state
}

bool UCombatState::OnDodgeIntent(UAnimMontage* Montage, const FVector2D& InputVector)
{
    if (!ownerChar || (playerCombatCancelComp && !playerCombatCancelComp->CanCancel())) return false;
    
    ICombatCmdInterface* iCombatCmd = ownerStateMachineComp->GetCombatCommands();
    if (!iCombatCmd) return false;

    if (playerCombatCancelComp) // If this is the player, cancel current action and attack
    {
        iCombatCmd->DodgeIntent(Montage, InputVector);
        return true;
    }
    else return false; // AI will always be set to none state before they attack, so they can't attack in this state
}

bool UCombatState::OnJumpStartIntent()
{
    Super::OnJumpStartIntent();

    if (!ownerChar) return false;

    ILocomotionCmdInterface* locoCMD = GetLocoCmd();

    if (playerCombatCancelComp && playerCombatCancelComp->CanCancel())
    {
        if (locoCMD) locoCMD->JumpStart();
        else ownerChar->Jump();
        return true;
    }
    else return false;
}

bool UCombatState::OnJumpStopIntent()
{
    if (!ownerChar) return false;

    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd()) locoCMD->JumpStop();
    else ownerChar->StopJumping();
    return true;
}