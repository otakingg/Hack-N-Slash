#include "AttackState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../../../Tags/AnimNotifyTags.h"
#include "../../../Characters/Shared/LocomotionComponent.h"
#include "../../../Tags/LocomotionTags.h"

void UAttackState::EnterState_Implementation()
{
    Super::EnterState_Implementation();
    if (locoComp) locoComp->AddMoveOverrideTag(OverrideTags::Lock);
}

void UAttackState::ExitState_Implementation()
{
    if (locoComp)
    {
        locoComp->RemoveMoveOverrideTag(OverrideTags::Lock);
        locoComp->RemoveMoveOverrideTag(OverrideTags::MoveStats);
    }
    bSetAirAtkStats = false;
    Super::ExitState_Implementation();
}

void UAttackState::OnAnimNotify(FGameplayTag NotifyTag)
{
    Super::OnAnimNotify(NotifyTag);

    if (NotifyTag.MatchesTagExact(StateMachineTags::AirAttacking) && !bSetAirAtkStats && locoComp && moveComp)
    {   
        bSetAirAtkStats = true;
        locoComp->AddMoveOverrideTag(OverrideTags::MoveStats);
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