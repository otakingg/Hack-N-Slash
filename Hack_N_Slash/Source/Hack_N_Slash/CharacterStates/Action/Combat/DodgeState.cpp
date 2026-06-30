#include "DodgeState.h"
#include "../../../Interfaces/CombatInstigator.h"
#include "../../../Characters/Shared/LocomotionComponent.h"
#include "../../../Tags/OverrideTags.h"

void UDodgeState::EnterState_Implementation()
{
    Super::EnterState_Implementation();

    if (iCmbtInst)
    {
        iCmbtInst->AddOverrideTag(OverrideTags::NoAtk);
        iCmbtInst->AddOverrideTag(OverrideTags::NoBlock);
        iCmbtInst->AddOverrideTag(OverrideTags::NoDodge);
        iCmbtInst->AddOverrideTag(OverrideTags::NoJump);
        iCmbtInst->AddOverrideTag(OverrideTags::NoMove);
    }
}

void UDodgeState::ExitState_Implementation()
{
    if (iCmbtInst)
    {
        iCmbtInst->RemoveOverrideTag(OverrideTags::NoAtk);
        iCmbtInst->RemoveOverrideTag(OverrideTags::NoBlock);
        iCmbtInst->RemoveOverrideTag(OverrideTags::NoDodge);
        iCmbtInst->RemoveOverrideTag(OverrideTags::NoJump);
        iCmbtInst->RemoveOverrideTag(OverrideTags::NoMove);
    }

    Super::ExitState_Implementation();
}

FGameplayTag UDodgeState::ResolvePlayerAction_Implementation(const FGameplayTag& PlayerAction, const FVector2D& InputVector)
{
    if (PlayerAction.MatchesTagExact(CharacterActionTags::BlockRelease)) return CharacterActionTags::None;
    else return PlayerAction;
}