#include "AirJumpStartState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../../Tags/AnimNotifyTags.h"
#include "../../Interfaces/LocomotionCmdInterface.h"
#include "../../Tags/LocomotionTags.h"

void UAirJumpStartState::EnterState()
{
    Super::EnterState();
    bImpulseApplied = false;

    // Only use tags for full lock behavior
    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd())
    {
        if (bLockMovementDuringJumpStart && lockedMoveScale <= KINDA_SMALL_NUMBER) locoCMD->AddMoveOverrideTag(TAG_Move_Override_Lock);
    }

    if (!bApplyImpulseOnNotify) ApplyJumpImpulseOnce();
}

void UAirJumpStartState::ExitState()
{
    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd()) locoCMD->RemoveMoveOverrideTag(TAG_Move_Override_Lock);
    Super::ExitState();
}

void UAirJumpStartState::OnAnimNotify(FGameplayTag NotifyTag)
{
    Super::OnAnimNotify(NotifyTag);

    if (bApplyImpulseOnNotify && !bImpulseApplied && NotifyTag.MatchesTagExact(TAG_Notify_StateMachine_Jump)) ApplyJumpImpulseOnce();
}

void UAirJumpStartState::ApplyJumpImpulseOnce()
{
    if (bImpulseApplied || !ownerChar || !moveComp) return;
    bImpulseApplied = true;

    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd())
    {
        if (bUseCharacterJumpFunction)
        {
            // Reset jump count to allow jump. Have to do this because UE consumes a jump internally when leaving ground
            ownerChar->JumpCurrentCount = 0;
            locoCMD->JumpStart();
        }
        else
        {
            const float JumpZ = (overrideJumpZVelocity > 0.f) ? overrideJumpZVelocity : moveComp->JumpZVelocity;
            FVector velocity {moveComp->Velocity.X, moveComp->Velocity.Y, JumpZ};
            locoCMD->LaunchCharacterHNS(velocity, false);
        }
    }
    ClearAirborneModeDelayed();
}