#include "JumpStartState.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../../../StateMachineComponent.h"
#include "../../Interfaces/LocomotionCmdInterface.h"

void UJumpStartState::EnterState()
{
    Super::EnterState();

    bImpulseApplied = false;

    if (!ownerChar || !moveComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UJumpStartState] EnterState: ownerChar or moveComp is null."));
        return;
    }

    // Apply impulse immediately unless animation-authoritative
    if (!bApplyImpulseOnNotify) ApplyJumpImpulseOnce();
}

void UJumpStartState::ExitState() { Super::ExitState(); }

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

    ApplyLookInputScaled(Look);
    return true;
}

bool UJumpStartState::OnMoveIntent(const FVector2D& Move, const FCommandContext& Ctx)
{
    if (!ownerChar) return false;

    // Record input context
    Super::OnMoveIntent(Move, Ctx);

    if (bLockMovementDuringJumpStart)
    {
        // Full lock: consume input and do nothing
        if (lockedMoveScale <= KINDA_SMALL_NUMBER) return true;

        // Partial drift
        ApplyMoveInputScaled(Move, lockedMoveScale);
        return true;
    }

    // Not locked, behave like locomotion (scale 1.0)
    ApplyMoveInputScaled(Move, 1.0f);
    return true;
}

void UJumpStartState::OnAnimNotify(FName NotifyName)
{
    Super::OnAnimNotify(NotifyName);

    if (bApplyImpulseOnNotify && !bImpulseApplied && NotifyName == takeoffNotifyName) ApplyJumpImpulseOnce();
}

void UJumpStartState::ApplyMoveInputScaled(const FVector2D& Move, float Scale)
{
    if (Scale <= 0.f) return;

    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd())
    {
        // You need this on the interface for Option B.
        // Player converts Move into AddMovementInput using control rotation
        // Enemy can treat it as desired strafe/forward intent
        locoCMD->AddMoveInputScaled(Move, Scale);
    }
}

void UJumpStartState::ApplyLookInputScaled(const FVector2D& Look)
{
    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd())
    {
        // You need this on the interface (or just SetLookIntent if you prefer).
        // This preserves your "turnRate/lookUpRate * DeltaSeconds" behavior.
        locoCMD->AddLookInputScaled(Look, turnRate, lookUpRate);
    }
}

void UJumpStartState::ApplyJumpImpulseOnce()
{
    if (bImpulseApplied || !ownerChar || !moveComp) return;
    bImpulseApplied = true;

    // Keep apex notifications if you use them
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