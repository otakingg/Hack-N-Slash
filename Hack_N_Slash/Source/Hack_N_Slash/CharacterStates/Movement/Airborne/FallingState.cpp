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

    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd())
    {
        if (!moveComp->IsFalling()) locoCMD->SetMovementModeCmd(MOVE_Falling);
        locoCMD->SetMoveProfileTag(ProfileTags::Falling);
    }
}