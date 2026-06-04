#include "FallingState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../../../Interfaces/LocomotionCmdInterface.h"
#include "../../../Tags/LocomotionTags.h"

bool UFallingState::CanEnterState_Implementation(const UCharacterState *PreviousState) const { return moveComp && moveComp->IsFalling(); }

void UFallingState::EnterState()
{
    Super::EnterState();

    if (!ownerChar) return;
    if (!moveComp) moveComp = ownerChar->GetCharacterMovement();
    if (!moveComp) return;

    // Baseline: we are airborne (container-level baseline)
    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd())
    {
        if (!moveComp->IsFalling()) locoCMD->SetMovementModeCmd(MOVE_Falling);

        // Stats-driven tuning via locomotion profile
        // Default move profile will be used unless another is passed in throuhg the "OnMoveIntent" functions
        locoCMD->SetMoveProfileTag(TAG_Move_Profile_Falling);
    }
}