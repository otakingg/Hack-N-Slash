#include "IsImmuneDamage.h"
#include "../../Characters/Shared/StatsComponent.h"

UIsImmuneDamage::UIsImmuneDamage()
{
    #if WITH_EDITORONLY_DATA
        NotifyColor = FColor::Green;
    #endif
}

void UIsImmuneDamage::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    UStatsComponent* statsComp = owner->FindComponentByClass<UStatsComponent>();
    if (statsComp) statsComp->SetDmgImmunity(true);
}

void UIsImmuneDamage::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    UStatsComponent* statsComp = owner->FindComponentByClass<UStatsComponent>();
    if (statsComp) statsComp->SetDmgImmunity(false);
}