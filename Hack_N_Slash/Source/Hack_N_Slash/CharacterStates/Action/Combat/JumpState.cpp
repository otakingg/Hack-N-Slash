#include "JumpState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../../../Tags/AnimNotifyTags.h"
#include "../../../Animation/AnimInstances/BaseCharAnimInstance.h"
#include "../../../Interfaces/LocomotionCmdInterface.h"
#include "../../../Combat/Player/PlayerCombatCancelComponent.h"
#include "../../../Characters/Shared/StateMachineComponent.h"

void UJumpState::OnJumpApexReached()
{
    if (!ownerChar) return;

    if (USkeletalMeshComponent* skeletalMeshComp = ownerChar->GetMesh())
    {
        if (UBaseCharAnimInstance* animInst = Cast<UBaseCharAnimInstance>(skeletalMeshComp->GetAnimInstance()))
        {
            UAnimMontage* activeMontage = animInst->GetCurrentActiveMontage();
            if (activeMontage) animInst->PlayMontageHNS(activeMontage, "Fall");
        }
    }
    
    if (ownerStateMachineComp) ownerStateMachineComp->ClearActionState();
}