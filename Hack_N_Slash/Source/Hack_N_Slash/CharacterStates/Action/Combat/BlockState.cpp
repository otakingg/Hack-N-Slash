#include "BlockState.h"
#include "../../../Interfaces/CombatCmdInterface.h"
#include "../../../Interfaces/LocomotionCmdInterface.h"
#include "../../../Tags/LocomotionTags.h"

void UBlockState::EnterState()
{
    Super::EnterState();
    if (ILocomotionCmdInterface* iLocoCmd = GetLocoCmd()) iLocoCmd->AddMoveOverrideTag(TAG_Move_Override_Lock);
}

void UBlockState::ExitState()
{
    if (ILocomotionCmdInterface* iLocoCmd = GetLocoCmd()) iLocoCmd->RemoveMoveOverrideTag(TAG_Move_Override_Lock);
    Super::ExitState();
}

bool UBlockState::OnBlockStopIntent()
{
    if (ICombatCmdInterface* iCombatCmd = GetCombatCmd())
    {
        iCombatCmd->BlockStopIntent();
        return true;
    }
    return false;
}