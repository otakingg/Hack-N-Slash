#include "AttackState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../../../Interfaces/CombatInstigator.h"
#include "../../../Tags/OverrideTags.h"

void UAttackState::EnterState_Implementation()
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

void UAttackState::ExitState_Implementation()
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

FGameplayTag UAttackState::ResolvePlayerAction_Implementation(const FGameplayTag& PlayerAction, const FVector2D& InputVector)
{
    if (PlayerAction.MatchesTagExact(CharacterActionTags::BlockRelease)) return CharacterActionTags::None;
    else return PlayerAction;
}