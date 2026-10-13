#include "JumpToSectionIfLowVelocity.h"
#include "GameFramework/MovementComponent.h"

UJumpToSectionIfLowVelocity::UJumpToSectionIfLowVelocity()
{
    #if WITH_EDITORONLY_DATA
        NotifyColor = FColor::White;
    #endif
}

void UJumpToSectionIfLowVelocity::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    UMovementComponent* moveComp = owner->FindComponentByClass<UMovementComponent>();
    if (!moveComp) return;

    if (moveComp->Velocity.Length() > velocity) return;

    UAnimInstance* animInst = MeshComp->GetAnimInstance();
    if (!animInst) return;

    UAnimMontage* currentMontage = animInst->GetCurrentActiveMontage();
    if (!currentMontage || !currentMontage->IsValidSectionName(section)) animInst->StopAllMontages(0.25f);
    else animInst->Montage_JumpToSection(section, currentMontage);
}