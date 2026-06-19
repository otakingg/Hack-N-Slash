#include "ClearHitActors.h"
#include "../Combat/Shared/CombatTraceComponent.h"

UClearHitActors::UClearHitActors()
{
    #if WITH_EDITORONLY_DATA
        NotifyColor = FColor::Red;
    #endif
}

void UClearHitActors::Notify(USkeletalMeshComponent *MeshComp, UAnimSequenceBase *Animation, const FAnimNotifyEventReference &EventReference)
{
    if (!MeshComp) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    if (UCombatTraceComponent* traceComp = owner->FindComponentByClass<UCombatTraceComponent>()) traceComp->ClearHitActors();
}