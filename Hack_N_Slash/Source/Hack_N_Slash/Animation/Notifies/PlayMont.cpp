#include "PlayMont.h"
#include "../AnimInstances/CombatantAnimInstance.h"

UPlayMont::UPlayMont()
{
    #if WITH_EDITORONLY_DATA
        NotifyColor = FColor::White;
    #endif
}

void UPlayMont::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    UCombatantAnimInstance* animInst = Cast<UCombatantAnimInstance>(MeshComp->GetAnimInstance());
    if (!animInst) return;

    if (!animInst->PlayMontage(montage, section)) animInst->StopAllMontages(0.25f);
}