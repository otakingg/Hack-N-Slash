#include "JumpStartState.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../../Interfaces/LocomotionCmdInterface.h"
#include "../../Tags/LocomotionTags.h"
#include "../../../StateMachineComponent.h"

void UJumpStartState::EnterState()
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

void UJumpStartState::ExitState()
{
    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd()) locoCMD->RemoveMoveOverrideTag(TAG_Move_Override_Lock);
    Super::ExitState();
}

bool UJumpStartState::OnLookIntent(const FCommandContext& Ctx, const FVector2D& Look)
{
    Super::OnLookIntent(Ctx, Look);

    // Eat look input entirely if not allowed
    if (!bAllowLookDuringJumpStart) return true;

    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd()) locoCMD->AddLookInputScaled(Look, turnRate, lookUpRate);

    return true;
}

bool UJumpStartState::OnMoveIntent(const FCommandContext& Ctx, const FVector2D& Move)
{
    Super::OnMoveIntent(Ctx, Move);

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

void UJumpStartState::OnAnimNotify(FName NotifyName)
{
    Super::OnAnimNotify(NotifyName);

    if (bApplyImpulseOnNotify && !bImpulseApplied && NotifyName == takeoffNotifyName) ApplyJumpImpulseOnce();
}

void UJumpStartState::ApplyJumpImpulseOnce()
{
    if (bImpulseApplied || !ownerChar || !moveComp) return;
    bImpulseApplied = true;
    moveComp->bNotifyApex = true;

    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd())
    {
        if (bUseCharacterJumpFunction)
        {
            locoCMD->JumpPressed();
            return;
        }

        const float JumpZ = (overrideJumpZVelocity > 0.f) ? overrideJumpZVelocity : moveComp->JumpZVelocity;

        // Add this optional method if you want Launch-style jumps data-driven:
        // virtual void LaunchUp(float JumpZ) = 0;
        // For now, you can either:
        // 1) extend the interface, or
        // 2) keep LaunchCharacter here (less pure Option B).

        // Recommended: extend interface.
        locoCMD->LaunchUp(JumpZ);
        return;
    }
}