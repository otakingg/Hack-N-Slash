#include "AirFloat.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../../Interfaces/CombatInstigator.h"
#include "../../Utility/Tags.h"

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
    if (iCmbtInst) iCmbtInst->AddTag(Tags::Status::MoveStatsOverride);

    if (ownerChar->IsJumpProvidingForce()) ownerChar->StopJumping();

    moveComp->GravityScale = gravity;

    // An aerial attack cancels upward momentum and limits downward momentum
    if (moveComp->Velocity.Z < minFallSpeed) moveComp->Velocity.Z = minFallSpeed;
    else if (moveComp->Velocity.Z > 0.0f) moveComp->Velocity.Z = 0.0f;
}

void UAirFloat::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    ACharacter* ownerChar = MeshComp->GetOwner<ACharacter>();
    if (!ownerChar) return;

    UCharacterMovementComponent* moveComp = ownerChar->GetCharacterMovement();
    if (!moveComp) return;

    ICombatInstigator* iCmbtInst = Cast<ICombatInstigator>(ownerChar);
    if (iCmbtInst) iCmbtInst->RemoveTag(Tags::Status::MoveStatsOverride);
    else moveComp->GravityScale = 1.0f;
}