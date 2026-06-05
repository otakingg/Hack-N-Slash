#include "DodgeState.h"
#include "../../../Interfaces/LocomotionCmdInterface.h"
#include "../../../Tags/LocomotionTags.h"

void UDodgeState::EnterState()
{
    Super::EnterState();
    if (ILocomotionCmdInterface* iLocoCmd = GetLocoCmd()) iLocoCmd->AddMoveOverrideTag(OverrideTags::Lock);
}

void UDodgeState::ExitState()
{
    if (ILocomotionCmdInterface* iLocoCmd = GetLocoCmd()) iLocoCmd->RemoveMoveOverrideTag(OverrideTags::Lock);
    Super::ExitState();
}