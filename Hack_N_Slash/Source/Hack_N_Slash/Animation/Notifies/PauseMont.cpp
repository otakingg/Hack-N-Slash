#include "PauseMont.h"

UPauseMont::UPauseMont()
{
    #if WITH_EDITORONLY_DATA
        NotifyColor = FColor::White;
    #endif
}

void UPauseMont::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    UAnimInstance* animInst = MeshComp->GetAnimInstance();
    if (!animInst) return;

    animInst->Montage_Pause(animInst->GetCurrentActiveMontage());
}