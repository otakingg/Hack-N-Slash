#include "LocomotionState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../../../Interfaces/LocomotionCmdInterface.h"
#include "../../../Tags/LocomotionTags.h"

bool ULocomotionState::CanEnterState_Implementation(const UCharacterState* PreviousState) const { return moveComp && moveComp->IsMovingOnGround(); }

void ULocomotionState::EnterState()
{
    Super::EnterState();

    if (!ownerChar) return;
    if (!moveComp) moveComp = ownerChar->GetCharacterMovement();
    if (!moveComp) return;

    // Baseline: we are grounded
    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd())
    {
        if (!moveComp->IsMovingOnGround()) locoCMD->SetMovementModeCmd(MOVE_Walking);
        locoCMD->MarkGroundedNow();
        locoCMD->SetMoveProfileTag(TAG_Move_Profile_Grounded);
    }
}

void ULocomotionState::ExitState()
{
    if (ILocomotionCmdInterface* locoCmd = GetLocoCmd()) locoCmd->MarkGroundedNow();
    Super::ExitState();
}