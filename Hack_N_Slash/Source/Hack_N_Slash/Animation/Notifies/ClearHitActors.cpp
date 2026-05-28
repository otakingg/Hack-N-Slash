#include "ClearHitActors.h"
#include "../Combat/Shared/CombatTraceComponent.h"

void UClearHitActors::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    if (UCombatTraceComponent* traceComp = owner->FindComponentByClass<UCombatTraceComponent>()) traceComp->ClearHitActors();
}