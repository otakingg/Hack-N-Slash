#include "ClearHitActorsN.h"
#include "../Combat/Shared/CombatTraceComponent.h"

void UClearHitActorsN::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    if (UCombatTraceComponent* traceComp = owner->FindComponentByClass<UCombatTraceComponent>()) traceComp->ClearHitActors();
}