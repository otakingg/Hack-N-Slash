#include "CombatState.h"
#include "GameFramework/Character.h"

#include "../../../Tags/CharacterStateTagNamespaces.h"
#include "../../../Interfaces/CombatCmdInterface.h"
#include "../../../Interfaces/LocomotionCmdInterface.h"
#include "../../../Combat/Player/PlayerCombatCancelComponent.h"
#include "../../../Characters/Shared/StateMachineComponent.h"

bool UCombatState::OnAttackIntent(const FVector2D &InputVector, EPlayerAction PlayerAction)
{
    if (!ownerChar || (playerCombatCancelComp && !playerCombatCancelComp->CanCancel(CombatTags::Attack))) return false;
    
    ICombatCmdInterface* iCombatCmd = ownerStateMachineComp->GetCombatCommands();
    if (!iCombatCmd) return false;

    if (playerCombatCancelComp) // If this is the player, cancel current action and attack
    {
        if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, FString::Printf(TEXT("[%s] Canceling Action"), *GetNameSafe(this)));
        iCombatCmd->AttackIntent(InputVector, PlayerAction);
        return true;
    }
    else return false; // AI will always be set to none state before they attack, so they can't attack in this state
}

bool UCombatState::OnDodgeIntent(const FVector2D& InputVector)
{
    if (!ownerChar || (playerCombatCancelComp && !playerCombatCancelComp->CanCancel(CombatTags::Dodge))) return false;
    
    ICombatCmdInterface* iCombatCmd = ownerStateMachineComp->GetCombatCommands();
    if (!iCombatCmd) return false;

    if (playerCombatCancelComp) // If this is the player, cancel current action and attack
    {
        if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, FString::Printf(TEXT("[%s] Canceling Action"), *GetNameSafe(this)));
        iCombatCmd->DodgeIntent(InputVector);
        return true;
    }
    else return false; // AI will always be set to none state before they attack, so they can't attack in this state
}

bool UCombatState::OnJumpStartIntent()
{
    Super::OnJumpStartIntent();

    if (!ownerChar) return false;

    ILocomotionCmdInterface* locoCMD = GetLocoCmd();

    if (playerCombatCancelComp && playerCombatCancelComp->CanCancel(CombatTags::Jump))
    {
        if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, FString::Printf(TEXT("[%s] Canceling Action"), *GetNameSafe(this)));
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