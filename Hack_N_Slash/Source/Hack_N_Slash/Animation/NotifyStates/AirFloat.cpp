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

    UCharacterMovementComponent* charMoveComp = ownerChar->GetCharacterMovement();
    if (!charMoveComp) return;

    ICombatInstigator* iCmbtInst = Cast<ICombatInstigator>(ownerChar);
    if (iCmbtInst) iCmbtInst->AddTag(Tags::Status::MoveStatsOverride);

    if (ownerChar->IsJumpProvidingForce()) ownerChar->StopJumping();

    charMoveComp->GravityScale = gravity;

    // An aerial attack cancels upward momentum and limits downward momentum
    if (charMoveComp->Velocity.Z < minFallSpeed) charMoveComp->Velocity.Z = minFallSpeed;
    else if (bRestrictUpwardVelocity && charMoveComp->Velocity.Z > 0.0f) charMoveComp->Velocity.Z = 0.0f;
}

void UAirFloat::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    ACharacter* ownerChar = MeshComp->GetOwner<ACharacter>();
    if (!ownerChar) return;

    UCharacterMovementComponent* charMoveComp = ownerChar->GetCharacterMovement();
    if (!charMoveComp) return;

    ICombatInstigator* iCmbtInst = Cast<ICombatInstigator>(ownerChar);
    if (iCmbtInst) iCmbtInst->RemoveTag(Tags::Status::MoveStatsOverride);
    else charMoveComp->GravityScale = 1.0f;
}