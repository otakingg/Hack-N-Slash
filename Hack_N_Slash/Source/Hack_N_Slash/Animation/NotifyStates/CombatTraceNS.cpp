#include "CombatTraceNS.h"
#include "../Combat/CombatTraceComponent.h"

void UCombatTraceNS::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
    if (!IsValid(MeshComp)) return;

    AActor* character = MeshComp->GetOwner();
    if (!IsValid(character)) return;

    traceComp = character->FindComponentByClass<UCombatTraceComponent>();
}

void UCombatTraceNS::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp || !traceComp) return;

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
    if (!traceComp) return;
    traceComp->ClearHitActors();
}