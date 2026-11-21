#include "AddTags.h"
#include "../../Interfaces/Combatant.h"

UAddTags::UAddTags()
{
    #if WITH_EDITORONLY_DATA
        NotifyColor = FColor::White;
    #endif
}

void UAddTags::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    ICombatant* iCombatant = Cast<ICombatant>(MeshComp->GetOwner());
    if (!iCombatant) return;

    for (const FGameplayTag& tag : Tags) iCombatant->AddTag(tag);
}

void UAddTags::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    ICombatant* iCombatant = Cast<ICombatant>(MeshComp->GetOwner());
    if (!iCombatant) return;

    for (const FGameplayTag& tag : Tags) iCombatant->RemoveTag(tag);
}