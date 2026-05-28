#include "SetDamageImmunity.h"
#include "../../Characters/Shared/StatsComponent.h"

void USetDamageImmunity::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    UStatsComponent* statsComp = owner->FindComponentByClass<UStatsComponent>();
    if (!statsComp) return;
    
    statsComp->SetDmgImmunity(startImmunity);
}

void USetDamageImmunity::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    UStatsComponent* statsComp = owner->FindComponentByClass<UStatsComponent>();
    if (!statsComp) return;
    
    statsComp->SetDmgImmunity(endImmunity);
}