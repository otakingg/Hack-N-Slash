#include "AttackState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../../../Tags/AnimNotifyTags.h"
#include "../../../Interfaces/LocomotionCmdInterface.h"
#include "../../../Tags/LocomotionTags.h"

void UAttackState::ExitState()
{
    if (ILocomotionCmdInterface* iLocoCmd = GetLocoCmd()) iLocoCmd->RemoveMoveOverrideTag(TAG_Move_Override_MoveStats);
    bSetAirAtkStats = false;
}

void UAttackState::OnAnimNotify(FGameplayTag NotifyTag)
{
    Super::OnAnimNotify(NotifyTag);

    if (!moveComp) return;

    if (NotifyTag.MatchesTagExact(TAG_Notify_StateMachine_AirAttacking))
    {
        ILocomotionCmdInterface* iLocoCmd = GetLocoCmd();
        if (!iLocoCmd || bSetAirAtkStats) return;
        iLocoCmd->AddMoveOverrideTag(TAG_Move_Override_MoveStats);
        moveComp->GravityScale = airAtkGravity;
        moveComp->Velocity.Z = 0.0f;
        //moveComp->StopMovementImmediately(); // Not sure if I want this instead
        //iLocoCmd->JumpStop(); // Not sure if I need to do this to
    }
}