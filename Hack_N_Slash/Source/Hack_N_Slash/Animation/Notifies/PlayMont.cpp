#include "PlayMont.h"
#include "../AnimInstances/BaseCharAnimInstance.h"

UPlayMont::UPlayMont()
{
    #if WITH_EDITORONLY_DATA
        NotifyColor = FColor::White;
    #endif
}

void UPlayMont::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    UBaseCharAnimInstance* animInst = Cast<UBaseCharAnimInstance>(MeshComp->GetAnimInstance());
    if (!animInst) return;

    if (!animInst->PlayMontageHNS(montage, section)) animInst->StopAllMontages(0.25f);
}