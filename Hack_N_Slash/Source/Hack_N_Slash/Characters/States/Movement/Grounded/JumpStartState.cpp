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

    ILocomotionCmdInterface* locoCMD = GetLocoCmd();
    if (locoCMD && bLockMovementDuringJumpStart) locoCMD->AddMoveOverrideTag(TAG_Move_Override_Lock);

    if (!bApplyImpulseOnNotify) ApplyJumpImpulseOnce();
}

void UJumpStartState::ExitState()
{
    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd()) locoCMD->RemoveMoveOverrideTag(TAG_Move_Override_Lock);
    Super::ExitState();
}

bool UJumpStartState::OnLookIntent(const FVector2D& Look)
{
    Super::OnLookIntent(Look);

    // Eat look input entirely if not allowed
    if (!bAllowLookDuringJumpStart) return true;

    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd())
    {
        locoCMD->AddLookInputScaled(Look, turnRate, lookUpRate);
        return true;
    }

    return false;
}

bool UJumpStartState::OnMoveIntent(const FVector2D& Move)
{
    Super::OnMoveIntent(Move);

    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd())
    {
        locoCMD->AddMoveInputScaled(Move);
        return true;
    }

    return false;
}

bool UJumpStartState::OnMoveIntent(const FGameplayTag& MoveProfile, AActor* Target, const FVector& Loc, float AcceptanceRadius)
{
    Super::OnMoveIntent(MoveProfile, Target, Loc, AcceptanceRadius);

    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd())
    {
        locoCMD->AddMoveInputScaled(Target, Loc, AcceptanceRadius);
        return true;
    }

    return false;
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