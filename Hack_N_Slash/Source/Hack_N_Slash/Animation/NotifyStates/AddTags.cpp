#include "AddTags.h"
#include "../../Interfaces/CombatInstigator.h"

UAddTags::UAddTags()
{
    #if WITH_EDITORONLY_DATA
        NotifyColor = FColor::White;
    #endif
}

void UAddTags::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    ICombatInstigator* iCmbtInst = Cast<ICombatInstigator>(MeshComp->GetOwner());
    if (!iCmbtInst) return;

    for (const FGameplayTag& tag : Tags) iCmbtInst->AddTag(tag);
}

void UAddTags::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    ICombatInstigator* iCmbtInst = Cast<ICombatInstigator>(MeshComp->GetOwner());
    if (!iCmbtInst) return;

    for (const FGameplayTag& tag : Tags) iCmbtInst->RemoveTag(tag);
}