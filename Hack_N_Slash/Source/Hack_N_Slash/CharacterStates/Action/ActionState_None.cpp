#include "ActionState_None.h"
#include "GameFramework/Character.h"
#include "../../Tags/CharacterStateTagNamespaces.h"
#include "../../Interfaces/CombatCmdInterface.h"
#include "../../Interfaces/LocomotionCmdInterface.h"
#include "../../Characters/Shared/StateMachineComponent.h"

bool UActionState_None::OnAttackIntent(const FVector2D& InputVector, EPlayerAction PlayerAction)
{
    if (ICombatCmdInterface* iCombatCmd = ownerStateMachineComp->GetCombatCommands())
    {
        iCombatCmd->AttackIntent(InputVector, PlayerAction);
        return true;
    }
    return false;
}

bool UActionState_None::OnDodgeIntent(UAnimMontage* Montage, const FVector2D& InputVector)
{
    if (ICombatCmdInterface* iCombatCmd = ownerStateMachineComp->GetCombatCommands())
    {
        iCombatCmd->DodgeIntent(Montage, InputVector);
        return true;
    }
    return false;
}

bool UActionState_None::OnJumpStartIntent()
{
    Super::OnJumpStartIntent();

    if (!ownerChar) return false;
    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd()) locoCMD->JumpStart();
    else ownerChar->Jump();
    return true;
}

bool UActionState_None::OnJumpStopIntent()
{
    if (!ownerChar) return false;

    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd()) locoCMD->JumpStop();
    else ownerChar->StopJumping();
    return true;
}