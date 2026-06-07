#include "PauseMontage.h"
#include "../AnimInstances/BaseCharAnimInstance.h"

void UPauseMontage::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    if (UAnimInstance* animInst = MeshComp->GetAnimInstance()) animInst->Montage_Pause();
}