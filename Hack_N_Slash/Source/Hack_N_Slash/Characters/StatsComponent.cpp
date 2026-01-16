// Fill out your copyright notice in the Description page of Project Settings
#include "StatsComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "../../Combat/Structs/FAtkHitData.h"

UStatsComponent::UStatsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UStatsComponent::BeginPlay()
{
	Super::BeginPlay();

	stats[EStat::Stagger] = stats[EStat::StaggerMax];
	bCanRegenStagger = false;
}

void UStatsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    RegenStat(stats[EStat::Stagger], stats[EStat::StaggerMax], bStaggerBroken? stats[EStat::StaggerRegen] * 0.75f : stats[EStat::StaggerRegen], bCanRegenStagger);
}

/************************************Private Functions************************************/
void UStatsComponent::RegenStat(float& val, float max, float rate, bool bCanRegen)
{
	if (!bCanRegen || val >= max) {return;}

	val = UKismetMathLibrary::FInterpTo_Constant(val, max, GetWorld()->DeltaTimeSeconds, rate);

	if (stats[EStat::Stagger] >= stats[EStat::StaggerMax]) {bStaggerBroken = false; bCanRegenStagger = false;}
}
/************************************Private Functions************************************/

/************************************Protected Functions************************************/
void UStatsComponent::EnableRegenStagger() {bCanRegenStagger = true;}
/************************************Protected Functions************************************/

/************************************Public Functions************************************/
void UStatsComponent::ApplyDamage(const FAtkHitData& hitData)
{
    if (stats[EStat::Health] <= 0.f) return;

    // APPLY HEALTH
	float dmgHP {hitData.baseDmgHP}; //Damage done to health

	float defense {stats[EStat::Defense]};
	float effectiveDefense {defense - (defense * hitData.penetration)}; //0 <= Penetration <= 1

	dmgHP *= (100.0f / (100.0f + effectiveDefense)); //Diminsihing returns formula
	stats[EStat::Health] = FMath::Clamp(stats[EStat::Health] - dmgHP, 0.f, stats[EStat::HealthMax]);
    OnHealthUpdateDelegate.Broadcast(GetStatPercentage(EStat::Health, EStat::HealthMax)); //Broadcast

    // APPLY STAGGER if it hasn't been broken yet
	if (!bStaggerBroken)
	{
		float dmgStagger {hitData.baseDmgStagger}; //Damage done to stagger meter
		dmgStagger *= (100.0f / (100.0f + stats[EStat::StaggerResist])); //Diminishing returns formula
		stats[EStat::Stagger] = FMath::Clamp(stats[EStat::Stagger] - dmgStagger, 0.f, stats[EStat::StaggerMax]);
	}

	if (stats[EStat::Stagger] <= 0.f)
	{
		bStaggerBroken = true;
		bCanRegenStagger = false;
		FLatentActionInfo staggerRegenInfo {0, 103, TEXT("EnableRegenStagger"), this};
		UKismetSystemLibrary::RetriggerableDelay(GetWorld(), stats[EStat::StaggerRegenDelay], staggerRegenInfo);
	}

    // DEATH CHECK
    if (stats[EStat::Health] <= 0.f) OnZeroHealthUpdateDelegate.Broadcast();
}
float UStatsComponent::GetStatPercentage(EStat current, EStat max) const {return stats[max] > 0.0f ? stats[current] / stats[max] : 0.0f;}
/************************************Public Functions************************************/