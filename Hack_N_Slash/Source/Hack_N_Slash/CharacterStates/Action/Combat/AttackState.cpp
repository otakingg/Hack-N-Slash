#include "AttackState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../../../Tags/AnimNotifyTags.h"
#include "../../../Interfaces/CombatInstigator.h"
#include "../../../Characters/Shared/LocomotionComponent.h"
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
    bSetAirAtkStats = false;
    Super::ExitState_Implementation();
}

void UAttackState::OnAnimNotify_Implementation(FGameplayTag NotifyTag)
{
    Super::OnAnimNotify_Implementation(NotifyTag);

    if (NotifyTag.MatchesTagExact(StateMachineTags::AirAttacking) && !bSetAirAtkStats && iCmbtInst && moveComp)
    {   
        bSetAirAtkStats = true;
        iCmbtInst->AddOverrideTag(OverrideTags::MoveStats);
        moveComp->GravityScale = airAtkGravity;
        moveComp->Velocity.Z = 0.0f;
        //moveComp->StopMovementImmediately(); // Not sure if I want this instead
        //iLocoCmd->JumpStop(); // Not sure if I need to do this to
    }
}

FGameplayTag UAttackState::ResolvePlayerAction_Implementation(const FGameplayTag& PlayerAction, const FVector2D& InputVector)
{
    if (PlayerAction.MatchesTagExact(CharacterActionTags::BlockRelease)) return CharacterActionTags::None;
    else return PlayerAction;
}