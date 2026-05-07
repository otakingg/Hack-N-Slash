#include "BlockBreakState.h"
#include "../../../Combat/Shared/CombatResolutionComponent.h"
//#include "../../../Interfaces/LocomotionCmdInterface.h"

void UBlockBreakState::ReceiveHit(const FAtkHitData &HitData)
{
    Super::ReceiveHit(HitData);
    
    if (!ownerChar || !combatResComp) return;

    combatResComp->PlayHitReaction(combatResComp->GetHitReactions().blockBreak);

    // Apply hit velocity
    //if (ILocomotionCmdInterface* locoCMD = GetLocoCmd()) locoCMD->LaunchCharacterHNS(HitData.motionVelocity, true, true, HitData.timeToStop, HitData.attacker);
}