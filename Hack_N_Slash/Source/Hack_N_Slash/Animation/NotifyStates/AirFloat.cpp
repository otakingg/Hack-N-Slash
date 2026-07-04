#include "AirFloat.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../../Interfaces/CombatInstigator.h"
#include "../../Tags/OverrideTags.h"

UAirFloat::UAirFloat()
{
    #if WITH_EDITORONLY_DATA
        NotifyColor = FColor(0, 255, 255);
    #endif
}

void UAirFloat::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    ACharacter* ownerChar = MeshComp->GetOwner<ACharacter>();
    if (!ownerChar) return;

    UCharacterMovementComponent* moveComp = ownerChar->GetCharacterMovement();
    if (!moveComp) return;

    ICombatInstigator* iCmbtInst = Cast<ICombatInstigator>(ownerChar);
    if (iCmbtInst) iCmbtInst->AddTag(OverrideTags::MoveStats);

    moveComp->GravityScale = gravity;

    if (moveComp->Velocity.Z < minFallSpeed)
    {
        FVector velocity = moveComp->Velocity;
        velocity.Z = minFallSpeed;
        moveComp->Velocity = velocity;
    }
}

void UAirFloat::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    ACharacter* ownerChar = MeshComp->GetOwner<ACharacter>();
    if (!ownerChar) return;

    UCharacterMovementComponent* moveComp = ownerChar->GetCharacterMovement();
    if (!moveComp) return;

    ICombatInstigator* iCmbtInst = Cast<ICombatInstigator>(ownerChar);
    if (iCmbtInst) iCmbtInst->RemoveTag(OverrideTags::MoveStats);
    else moveComp->GravityScale = 1.0f;
}