#include "CombatTrace.h"
#include "../Combat/Shared/CombatTraceComponent.h"

UCombatTrace::UCombatTrace()
{
    #if WITH_EDITORONLY_DATA
        NotifyColor = FColor::Red;
    #endif
}

void UCombatTrace::Notify(USkeletalMeshComponent *MeshComp, UAnimSequenceBase *Animation, const FAnimNotifyEventReference &EventReference)
{
    if (!MeshComp) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    UCombatTraceComponent* traceComp = owner->FindComponentByClass<UCombatTraceComponent>();
	if (!traceComp) return;

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