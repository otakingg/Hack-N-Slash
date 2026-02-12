// Fill out your copyright notice in the Description page of Project Settings
#include "StatsComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "../../Structs/FAtkHitData.h"

UStatsComponent::UStatsComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UStatsComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UStatsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

// -------------------------- Queries --------------------------
bool UStatsComponent::HasStat(EStat Stat) const { return stats.Contains(Stat); }

float UStatsComponent::GetStat(EStat Stat) const
{
	if (const float* val = stats.Find(Stat)) return *val;
	return 0.f;
}

float UStatsComponent::GetStatPercentage(EStat Current, EStat Max) const
{
	float current {GetStat(Current)};
	float max {GetStat(Max)};
	return max > 0.0f ? current / max : 0.0f;
}

// -------------------------- Regen --------------------------
void UStatsComponent::RegenStat(float DeltaTime, float& Val, float Max, float Rate, bool bCanRegen)
{
	if (!bCanRegen || Val >= Max) {return;}

	Val = UKismetMathLibrary::FInterpTo_Constant(Val, Max, DeltaTime, Rate);
}

// -------------------------- Damage Application --------------------------
void UStatsComponent::RecieveHit(const FAtkHitData& hitData)
{
    if (GetStat(EStat::Health) <= 0.0f || GetStat(EStat::HealthMax) <= 0.0f) return;

    // APPLY HEALTH
	float dmgHP {hitData.baseDmgHP}; //Damage done to health

	const float pen {FMath::Clamp(hitData.penetration, 0.0f, 1.0f)};
	float defense {GetStat(EStat::Defense)};
	float effectiveDefense {defense - (defense * pen)};

	dmgHP *= (100.0f / (100.0f + effectiveDefense)); //Diminsihing returns formula
	stats[EStat::Health] = FMath::Clamp(stats[EStat::Health] - dmgHP, 0.f, stats[EStat::HealthMax]);
    OnHealthUpdateDelegate.Broadcast(GetStatPercentage(EStat::Health, EStat::HealthMax)); //Broadcast

    // DEATH CHECK
    if (stats[EStat::Health] <= 0.f) OnZeroHealthUpdateDelegate.Broadcast();
}