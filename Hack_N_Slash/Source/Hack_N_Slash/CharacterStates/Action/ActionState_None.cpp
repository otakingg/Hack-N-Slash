#include "ActionState_None.h"
#include "GameFramework/Character.h"
#include "../../Interfaces/CombatCmdInterface.h"
#include "../../Characters/Shared/StateMachineComponent.h"

bool UActionState_None::OnAttackIntent(const FVector2D& InputVector)
{
    if (ICombatCmdInterface* iCombatCmd = ownerStateMachineComp->GetCombatCommands())
    {
        iCombatCmd->AttackIntent(InputVector);
        return true;
    }
    return false;
}

bool UActionState_None::OnBlockStartIntent()
{
    return false;
}

bool UActionState_None::OnBlockStopIntent()
{
    return false;
}

bool UActionState_None::OnDodgeIntent(const FVector2D& InputVector)
{
    return false;
}