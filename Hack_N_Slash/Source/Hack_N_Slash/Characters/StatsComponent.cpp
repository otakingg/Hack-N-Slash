// Fill out your copyright notice in the Description page of Project Settings
#include "StatsComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

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
void UStatsComponent::ApplyDamage(float HealthDmg, float Penetration)
{
    if (GetStat(EStat::Health) <= 0.0f || GetStat(EStat::HealthMax) <= 0.0f) return;

    // Apply health damage
	HealthDmg = FMath::Clamp(HealthDmg, 0.0f, FLT_MAX);
	Penetration = FMath::Clamp(Penetration, 0.0f, 1.0f);
	
	float defense {GetStat(EStat::Defense)};
	float effectiveDefense {defense - (defense * Penetration)};

	HealthDmg *= (100.0f / (100.0f + effectiveDefense)); //Diminsihing returns formula
	stats[EStat::Health] = FMath::Clamp(stats[EStat::Health] - HealthDmg, 0.0f, stats[EStat::HealthMax]);

	// Events
    OnHealthUpdateDel.Broadcast(GetStatPercentage(EStat::Health, EStat::HealthMax));
    if (stats[EStat::Health] <= 0.f) OnZeroHealthUpdateDel.Broadcast();
}