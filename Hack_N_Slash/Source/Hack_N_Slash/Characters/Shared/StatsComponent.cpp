#include "StatsComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

UStatsComponent::UStatsComponent() { PrimaryComponentTick.bCanEverTick = false; }

void UStatsComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UStatsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

// -------------------------- Queries --------------------------
bool UStatsComponent::HasStat(EStat Stat) const { return stats.Contains(Stat); }

float UStatsComponent::GetStat(EStat Stat) const
{
	if (HasStat(Stat)) return stats[Stat];
	else return 0.0f;
}

float UStatsComponent::GetStatPercentage(EStat Current, EStat Max) const
{
	float current = GetStat(Current);
	float max = GetStat(Max);
	return max > 0.0f ? current / max : 0.0f;
}

void UStatsComponent::SetStat(EStat Stat, float Value) { if (HasStat(Stat)) stats[Stat] = Value; }

// -------------------------- Damage Application --------------------------
float UStatsComponent::ApplyDamage(float Damage, float Penetration)
{
    if (bDmgImmune || GetStat(EStat::Health) <= 0.0f || GetStat(EStat::HealthMax) <= 0.0f) return 0.0f;

    // Apply health damage
	Damage = FMath::Clamp(Damage, 0.0f, FLT_MAX);
	Penetration = FMath::Clamp(Penetration, 0.0f, 1.0f);
	
	float defense = GetStat(EStat::Defense);
	float effectiveDefense = defense - (defense * Penetration); // Apply penetration

	Damage *= (100.0f / (100.0f + effectiveDefense)); //Diminsihing returns formula
	stats[EStat::Health] = FMath::Clamp(stats[EStat::Health] - Damage, 0.0f, stats[EStat::HealthMax]);

	// Events
    OnHealthUpdate.Broadcast(GetStatPercentage(EStat::Health, EStat::HealthMax));
    if (stats[EStat::Health] <= 0.f) OnZeroHealthUpdate.Broadcast();
	return Damage;
}