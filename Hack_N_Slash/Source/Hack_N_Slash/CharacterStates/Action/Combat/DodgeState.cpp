#include "DodgeState.h"
#include "../../../Interfaces/CombatInstigator.h"
#include "../../../Characters/Shared/LocomotionComponent.h"
#include "../../../Tags/OverrideTags.h"

void UDodgeState::EnterState_Implementation()
{
    Super::EnterState_Implementation();

    if (iCmbtInst)
    {
        iCmbtInst->AddTag(OverrideTags::NoAtk);
        iCmbtInst->AddTag(OverrideTags::NoBlock);
        iCmbtInst->AddTag(OverrideTags::NoDodge);
        iCmbtInst->AddTag(OverrideTags::NoJump);
        iCmbtInst->AddTag(OverrideTags::NoMove);
    }
}

void UDodgeState::ExitState_Implementation()
{
    if (iCmbtInst)
    {
        iCmbtInst->RemoveTag(OverrideTags::NoAtk);
        iCmbtInst->RemoveTag(OverrideTags::NoBlock);
        iCmbtInst->RemoveTag(OverrideTags::NoDodge);
        iCmbtInst->RemoveTag(OverrideTags::NoJump);
        iCmbtInst->RemoveTag(OverrideTags::NoMove);
    }

    Super::ExitState_Implementation();
}

FGameplayTag UDodgeState::ResolvePlayerAction_Implementation(const FGameplayTag& PlayerAction, const FVector2D& InputVector)
{
    if (PlayerAction.MatchesTagExact(CharacterActionTags::BlockRelease)) return CharacterActionTags::None;
    else return PlayerAction;
}