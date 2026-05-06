#include "JumpState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../../../Tags/AnimNotifyTags.h"
#include "../../../Interfaces/CharAnimInterface.h"
#include "../../../Interfaces/LocomotionCmdInterface.h"
#include "../../../Combat/Player/PlayerCombatCancelComponent.h"
#include "../../../Characters/Shared/StateMachineComponent.h"

void UJumpState::OnJumpApexReached()
{
    if (!ownerChar) return;

    if (USkeletalMeshComponent* skeletalMeshComp = ownerChar->GetMesh())
    {
        if (ICharAnimInterface* iAnimInst = Cast<ICharAnimInterface>(skeletalMeshComp->GetAnimInstance()))
        {
            UAnimMontage* activeMontage = iAnimInst->GetActiveMontage();
            if (activeMontage) iAnimInst->PlayMontageHNS(activeMontage, "Fall");
        }
    }
    
    if (ownerStateMachineComp) ownerStateMachineComp->ClearActionState();
}