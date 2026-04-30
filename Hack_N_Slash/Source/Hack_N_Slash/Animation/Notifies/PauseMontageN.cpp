#include "PauseMontageN.h"
#include "../../Interfaces/CharAnimInterface.h"

void UPauseMontageN::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    ICharAnimInterface* iCharAnimInst = Cast<ICharAnimInterface>(MeshComp->GetAnimInstance());
    if (!iCharAnimInst) return;

    iCharAnimInst->PauseMontageHNS();
}