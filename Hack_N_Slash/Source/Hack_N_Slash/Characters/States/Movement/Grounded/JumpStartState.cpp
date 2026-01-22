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

    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd())
    {
        if (bLockMovementDuringJumpStart)
        {
            if (lockedMoveScale <= KINDA_SMALL_NUMBER) locoCMD->AddMoveOverrideTag(TAG_Move_Override_Lock);
            else
            {
                // For now, use Slow as "partial drift".
                // Later we can support configurable slow percent with a tag->float map.
                locoCMD->AddMoveOverrideTag(TAG_Move_Override_Slow);
            }
        }
    }

    if (!bApplyImpulseOnNotify) ApplyJumpImpulseOnce();
}

void UJumpStartState::ExitState()
{
    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd())
    {
        locoCMD->RemoveMoveOverrideTag(TAG_Move_Override_Lock);
        locoCMD->RemoveMoveOverrideTag(TAG_Move_Override_Slow);
    }

    Super::ExitState();
}

// Allow JumpStart during coyote time
bool UJumpStartState::CanEnterGroundedMode_Implementation(const UCharacterState* Prev) const
{
    if (!ownerChar || !moveComp) return false;

    const float Now = ownerChar->GetWorld()->GetTimeSeconds();
    return moveComp->IsMovingOnGround() || ((Now - lastGroundedTime) <= coyoteSeconds);
}

bool UJumpStartState::OnLookIntent(const FVector2D& Look, const FCommandContext& Ctx)
{
    if (!ownerChar) return false;

    // Record input context
    Super::OnLookIntent(Look, Ctx);

	// If you want to "eat" look while locked, return true.
    // If you want camera to still work from other systems, return false.
    if (!bAllowLookDuringJumpStart) return true;

    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd())
    {
        // You need this on the interface (or just SetLookIntent if you prefer).
        // This preserves your "turnRate/lookUpRate * DeltaSeconds" behavior.
        locoCMD->AddLookInputScaled(Look, turnRate, lookUpRate);
    }

    return false;
}

bool UJumpStartState::OnMoveIntent(const FVector2D& Move, const FCommandContext& Ctx)
{
    Super::OnMoveIntent(Move, Ctx);

    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd())
    {
        // If locked: locomotion ignores it anyway
        // If slow: this allows drift while speed is reduced
        locoCMD->AddMoveInput(Move);
        return true;
    }

    return true; // Consume regardless
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