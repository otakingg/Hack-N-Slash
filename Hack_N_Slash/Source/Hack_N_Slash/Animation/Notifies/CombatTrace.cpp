#include "CombatTrace.h"
#include "../../Combat/Shared/CombatTraceComponent.h"

UCombatTrace::UCombatTrace()
{
    #if WITH_EDITORONLY_DATA
        NotifyColor = FColor::Red;
    #endif
}

void UCombatTrace::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    UCombatTraceComponent* traceComp = owner->FindComponentByClass<UCombatTraceComponent>();
    if (!traceComp) return;

    FAtkHitData hitData = FAtkHitData::FAtkHitData(); // Create hit data

    // Source
    // Both the attacker and damager are the owner in this case
    hitData.attacker = owner;
    hitData.damager = owner;

    // Tags
	hitData.attackMotionTag = attackMotionTag;
	hitData.attackTypeTag = attackTypeTag;
    hitData.elementTags = elementTags;

    // Special
    hitData.attackIntent = attackIntent;
    hitData.bArmorBreaker = bArmorBreaker;
    hitData.bIsCounterFollowUp = bIsCounterFollowUp;

    // Numbers
    hitData.aggroBuildup = aggroBuildup;

    // Knockback
    hitData.bAdditive = bAdditive;
    hitData.localDir = localDir;
    hitData.distance = distance;
    hitData.duration = duration;
    hitData.velocityOnFinishMode = velocityOnFinishMode;
    hitData.velocityOnFinish = velocityOnFinish;
    hitData.clampVelocityOnFinish = clampVelocityOnFinish;
    hitData.strengthOverTime = strengthOverTime;

    traceComp->BuildHitData(hitData);

    switch (traceType)
    {
    case ETraceTypeN::Distance:
        traceComp->DistanceTrace(traceRadius, traceDistance, traceOffset);
        break;
    
    case ETraceTypeN::Socket:
        traceComp->SocketTrace(MeshComp, sockets, traceRadius);
        break;
    
    default:
        break;
    }
}