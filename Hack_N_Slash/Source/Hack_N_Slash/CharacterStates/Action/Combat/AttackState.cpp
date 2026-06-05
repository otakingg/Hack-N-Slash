#include "AttackState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../../../Tags/AnimNotifyTags.h"
#include "../../../Interfaces/LocomotionCmdInterface.h"
#include "../../../Tags/LocomotionTags.h"

void UAttackState::EnterState()
{
    Super::EnterState();
    if (ILocomotionCmdInterface* iLocoCmd = GetLocoCmd()) iLocoCmd->AddMoveOverrideTag(OverrideTags::Lock);
}

void UAttackState::ExitState()
{
    if (ILocomotionCmdInterface* iLocoCmd = GetLocoCmd())
    {
        iLocoCmd->RemoveMoveOverrideTag(OverrideTags::Lock);
        iLocoCmd->RemoveMoveOverrideTag(OverrideTags::MoveStats);
    }
    bSetAirAtkStats = false;
    Super::ExitState();
}

void UAttackState::OnAnimNotify(FGameplayTag NotifyTag)
{
    Super::OnAnimNotify(NotifyTag);

    if (!moveComp) return;

    if (NotifyTag.MatchesTagExact(StateMachineTags::AirAttacking) && !bSetAirAtkStats)
    {
        ILocomotionCmdInterface* iLocoCmd = GetLocoCmd();
        if (!iLocoCmd) return;
        
        bSetAirAtkStats = true;
        iLocoCmd->AddMoveOverrideTag(OverrideTags::MoveStats);
        moveComp->GravityScale = airAtkGravity;
        moveComp->Velocity.Z = 0.0f;
        //moveComp->StopMovementImmediately(); // Not sure if I want this instead
        //iLocoCmd->JumpStop(); // Not sure if I need to do this to
    }
}