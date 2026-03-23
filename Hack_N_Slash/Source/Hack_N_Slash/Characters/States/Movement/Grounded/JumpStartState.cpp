#include "JumpStartState.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

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

void UJumpStartState::OnAnimNotify(FName NotifyName)
{
    Super::OnAnimNotify(NotifyName);

    if (bApplyImpulseOnNotify && !bImpulseApplied && NotifyName == jumpNotifyName) ApplyJumpImpulseOnce();
}

void UJumpStartState::ApplyJumpImpulseOnce()
{
    if (bImpulseApplied || !ownerChar || !moveComp) return;
    bImpulseApplied = true;
    moveComp->bNotifyApex = true;

    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd())
    {
        if (bUseCharacterJumpFunction) locoCMD->JumpPressed();
        else
        {
            const float JumpZ = (overrideJumpZVelocity > 0.f) ? overrideJumpZVelocity : moveComp->JumpZVelocity;
            FVector velocity {moveComp->Velocity.X, moveComp->Velocity.Y, JumpZ};
            locoCMD->LaunchCharacterHNS(velocity, false);
        }
    }
}