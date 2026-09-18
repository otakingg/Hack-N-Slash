#include "CombatTrace.h"
#include "Kismet/KismetMathLibrary.h"

#include "../../Combat/Shared/CombatResolutionComponent.h"
#include "../../Combat/Shared/CombatTraceComponent.h"
#include "../../Characters/Shared/StatsComponent.h"

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

    UStatsComponent* statsComp = owner->FindComponentByClass<UStatsComponent>();
    if (!statsComp) return;

    UCombatTraceComponent* traceComp = owner->FindComponentByClass<UCombatTraceComponent>();
    if (!traceComp) return;

    UCombatResolutionComponent* combatResComp = owner->FindComponentByClass<UCombatResolutionComponent>();

    FAtkHitData hitData = FAtkHitData::FAtkHitData(); // Create hit data

    // Source
    // Both the attacker and damager are the owner in this case
    hitData.attacker = owner;
    hitData.damager = owner;

    // Tags
	hitData.attackMotion = attackMotion;
	hitData.attackType = attackType;
    hitData.elements = elements;

    // Special
    hitData.attackIntent = attackIntent;
    hitData.bArmorBreaker = bArmorBreaker;
    hitData.bIsCounterFollowUp = bIsCounterFollowUp;

    // Numbers
    hitData.aggroBuildup = aggroBuildup;
	hitData.dmg = statsComp->GetStat(EStat::Strength) * dmgMult;
	hitData.penetration = statsComp->GetStat(EStat::Penetration);
	hitData.poise = combatResComp ? combatResComp->GetPoiseCalc() : 0;

	float critRate = statsComp->GetStat(EStat::CritRate);
	if (critRate > 0.0f && UKismetMathLibrary::RandomFloatInRange(0.f, 1.f) <= critRate) hitData.dmg *= statsComp->GetStat(EStat::CritDmg);

    // Knockback
    hitData.knockBackType = knockBackType;
    hitData.velocityOnFinishMode = velocityOnFinishMode;
    hitData.velocityOnFinish = velocityOnFinish;
    hitData.clampVelocityOnFinish = clampVelocityOnFinish;
    hitData.bAdditive = bAdditive;
    hitData.localDir = localDir.GetSafeNormal();
    hitData.distance = distance;
    hitData.duration = duration;
    hitData.strengthOverTime = strengthOverTime;
    hitData.bRestrictSpeedToExpected = bRestrictSpeedToExpected;
    hitData.moveToLoc = owner->GetActorLocation() + owner->GetActorRotation().RotateVector(moveToOffset);
    hitData.gbExtraBounceHeight = gbExtraBounceHeight;
    hitData.bGBAdditive = bGBAdditive;
    hitData.gbSOT = gbSOT;
    hitData.gbCVOF = gbCVOF;

    // Feedback
    hitData.hitSFX = hitSFX;
    hitData.hitVFX = hitVFX;

    traceComp->SetHitData(hitData);

    switch (traceType)
    {
    case ETraceTypeN::Forward:
        traceComp->ForwardTrace(traceRadius, traceDistance, traceOffset);
        break;
    
    case ETraceTypeN::Socket:
        traceComp->SocketTrace(MeshComp, sockets, traceRadius);
        break;
    
    default:
        break;
    }
}