#include "CombatState.h"
#include "GameFramework/Character.h"

#include "../../../Tags/CharacterStateTagNamespaces.h"
#include "../../../Interfaces/CombatCmdInterface.h"
#include "../../../Interfaces/LocomotionCmdInterface.h"
#include "../../../Combat/Player/PlayerCombatCancelComponent.h"

/* AI will always be set to none state before they can do an action attack, so these functions will always return false for them. Jump stop is an exception */

void UCombatState::EnterState()
{
    Super::EnterState();
    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd()) locoCMD->ClearRootMotionSource();
}

bool UCombatState::OnAttackIntent(const FVector2D &InputVector, EPlayerAction PlayerAction)
{
    if (!ownerChar || !playerCombatCancelComp || !playerCombatCancelComp->CanCancel(CombatTags::Attack)) return false;
    
    ICombatCmdInterface* iCombatCmd = GetCombatCmd();
    if (!iCombatCmd) return false;

    if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, FString::Printf(TEXT("[%s] Canceling Action"), *GetNameSafe(this)));
    iCombatCmd->AttackIntent(InputVector, PlayerAction);
    return true;
}

bool UCombatState::OnDodgeIntent(const FVector2D& InputVector)
{
    if (!ownerChar || !playerCombatCancelComp || !playerCombatCancelComp->CanCancel(CombatTags::Dodge)) return false;
    
    ICombatCmdInterface* iCombatCmd = GetCombatCmd();
    if (!iCombatCmd) return false;

    if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, FString::Printf(TEXT("[%s] Canceling Action"), *GetNameSafe(this)));
    iCombatCmd->DodgeIntent(InputVector);
    return true;
}

bool UCombatState::OnJumpStartIntent()
{
    if (!ownerChar || !playerCombatCancelComp || !playerCombatCancelComp->CanCancel(CombatTags::Jump)) return false;

    ILocomotionCmdInterface* locoCMD = GetLocoCmd();

    if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, FString::Printf(TEXT("[%s] Canceling Action"), *GetNameSafe(this)));
    if (locoCMD) locoCMD->JumpStart();
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