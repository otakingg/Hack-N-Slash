#include "JumpStartState.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../../Tags/AnimNotifyTags.h"
#include "../../Interfaces/LocomotionCmdInterface.h"
#include "../../Tags/LocomotionTags.h"

void UJumpStartState::EnterState()
{
    Super::EnterState();
    bImpulseApplied = false;

    ILocomotionCmdInterface* locoCMD = GetLocoCmd();
    if (locoCMD && bLockMovementDuringJumpStart) locoCMD->AddMoveOverrideTag(TAG_Move_Override_Lock);

    if (!bApplyImpulseOnNotify) ApplyJumpImpulseOnce();
}

void UJumpStartState::ExitState()
{
    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd()) locoCMD->RemoveMoveOverrideTag(TAG_Move_Override_Lock);
    Super::ExitState();
}

void UJumpStartState::OnAnimNotify(FGameplayTag NotifyTag)
{
    Super::OnAnimNotify(NotifyTag);

    if (bApplyImpulseOnNotify && !bImpulseApplied && NotifyTag.MatchesTagExact(TAG_Notify_StateMachine_Jump)) ApplyJumpImpulseOnce();
}

void UJumpStartState::ApplyJumpImpulseOnce()
{
    if (bImpulseApplied || !ownerChar || !moveComp) return;
    bImpulseApplied = true;

    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd())
    {
        if (bUseCharacterJumpFunction) locoCMD->JumpStart();
        else
        {
            const float JumpZ = (overrideJumpZVelocity > 0.f) ? overrideJumpZVelocity : moveComp->JumpZVelocity;
            FVector velocity {moveComp->Velocity.X, moveComp->Velocity.Y, JumpZ};
            locoCMD->LaunchCharacterHNS(velocity, false);
        }
    }
}