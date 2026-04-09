#include "CombatTraceN.h"
#include "../Combat/Shared/CombatTraceComponent.h"

void UCombatTraceN::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!IsValid(MeshComp)) {return;}

    AActor* owner = MeshComp->GetOwner();
    if (!IsValid(owner)) {return;}

    UCombatTraceComponent* traceComp = owner->FindComponentByClass<UCombatTraceComponent>();
	if (!IsValid(traceComp)) {return;}

    switch (traceType)
    {
    case ETraceTypeN::Distance:
        traceComp->DistanceTrace(traceRadius, traceDistance, traceOffset, hitData);
        break;
    
    case ETraceTypeN::Socket:
        traceComp->SocketTrace(MeshComp, sockets, traceRadius, hitData);
        break;
    
    default:
        break;
    }
}