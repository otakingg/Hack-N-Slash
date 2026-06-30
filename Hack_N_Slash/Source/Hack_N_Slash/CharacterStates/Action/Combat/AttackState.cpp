#include "AttackState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../../../Interfaces/CombatInstigator.h"
#include "../../../Tags/OverrideTags.h"

void UAttackState::EnterState_Implementation()
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

void UAttackState::ExitState_Implementation()
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

FGameplayTag UAttackState::ResolvePlayerAction_Implementation(const FGameplayTag& PlayerAction, const FVector2D& InputVector)
{
    if (PlayerAction.MatchesTagExact(CharacterActionTags::BlockRelease)) return CharacterActionTags::None;
    else return PlayerAction;
}