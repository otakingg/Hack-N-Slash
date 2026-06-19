#include "CombatTraceNS.h"
#include "../Combat/Shared/CombatTraceComponent.h"

UCombatTraceNS::UCombatTraceNS()
{
    #if WITH_EDITORONLY_DATA
        NotifyColor = FColor::Red;
    #endif
}

void UCombatTraceNS::NotifyTick(USkeletalMeshComponent *MeshComp, UAnimSequenceBase *Animation, float FrameDeltaTime, const FAnimNotifyEventReference &EventReference)
{
    if (!MeshComp) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    UCombatTraceComponent* traceComp = owner->FindComponentByClass<UCombatTraceComponent>();
    if (!traceComp) return;

    hitData.localDir = hitData.localDir.GetSafeNormal();
    
    switch (traceType)
    {
    case ETraceTypeNS::Distance:
        traceComp->DistanceTrace(traceRadius, traceDistance, traceOffset, hitData);
        break;
    
    case ETraceTypeNS::Socket:
        traceComp->SocketTrace(MeshComp, sockets, traceRadius, hitData);
        break;
    
    default:
        break;
    }
}

void UCombatTraceNS::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    UCombatTraceComponent* traceComp = owner->FindComponentByClass<UCombatTraceComponent>();
    if (!traceComp) return;

    traceComp->ClearHitActors();
}