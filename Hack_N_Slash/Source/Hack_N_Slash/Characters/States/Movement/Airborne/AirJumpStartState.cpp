#include "AirJumpStartState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../../Interfaces/LocomotionCmdInterface.h"
#include "../../Tags/LocomotionTags.h"
#include "../../../StateMachineComponent.h"

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

bool UAirJumpStartState::OnLookIntent(const FVector2D& Look, const FCommandContext& Ctx)
{
    Super::OnLookIntent(Look, Ctx);

    // Eat look input entirely if not allowed
    if (!bAllowLookDuringJumpStart) return true;

    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd()) locoCMD->AddLookInputScaled(Look, turnRate, lookUpRate);

    return true;
}

bool UAirJumpStartState::OnMoveIntent(const FVector2D& Move, const FCommandContext& Ctx)
{
    Super::OnMoveIntent(Move, Ctx);

    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd())
    {
        if (bLockMovementDuringJumpStart)
        {
            // Full lock: tag already blocks movement; consume
            if (lockedMoveScale <= KINDA_SMALL_NUMBER) return true;

            // Partial drift: scale input directly (respects lockedMoveScale)
            locoCMD->AddMoveInputScaled(Move * lockedMoveScale);
            return true;
        }

        // Not locked: normal movement
        locoCMD->AddMoveInputScaled(Move);
    }

    return true;
}

void UAirJumpStartState::OnAnimNotify(FName NotifyName)
{
    Super::OnAnimNotify(NotifyName);

    if (bApplyImpulseOnNotify && !bImpulseApplied && NotifyName == takeoffNotifyName) ApplyJumpImpulseOnce();
}

void UAirJumpStartState::ApplyJumpImpulseOnce()
{
    if (bImpulseApplied || !ownerChar || !moveComp) return;
    bImpulseApplied = true;
    moveComp->bNotifyApex = true;

    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd())
    {
        if (bUseCharacterJumpFunction)
        {
            // Reset jump count to allow jump. Have to do this because UE consumes a jump internally when leaving ground
            ownerChar->JumpCurrentCount = 0;
            locoCMD->JumpPressed();
        }
        else
        {
            const float JumpZ = (overrideJumpZVelocity > 0.f) ? overrideJumpZVelocity : moveComp->JumpZVelocity;
            locoCMD->LaunchUp(JumpZ);
        }
    }
    //if (ownerStateMachineComp) ownerStateMachineComp->ClearAirborneMode();
}