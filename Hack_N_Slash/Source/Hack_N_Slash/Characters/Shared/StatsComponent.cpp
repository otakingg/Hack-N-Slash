#include "StatsComponent.h"

UStatsComponent::UStatsComponent() { PrimaryComponentTick.bCanEverTick = false; }

// -------------------------- Queries --------------------------
// bool UStatsComponent::HasStat(EStat Stat) const { return stats.Contains(Stat); }

float UStatsComponent::GetStat(EStat Stat) const
{
	if (stats.Contains(Stat)) return stats[Stat];
	else return -1.0f;
}

float UStatsComponent::GetStatPercentage(EStat Current, EStat Max) const
{
	float current = GetStat(Current);
	float max = GetStat(Max);

	if (current <= 0.0f || max <= 0.0f) return 0.0f;
	else return current / max;
}

void UStatsComponent::SetStat(EStat Stat, float Value) { if (stats.Contains(Stat)) stats[Stat] = Value; }

/* -------------------------- Damage Application -------------------------- */
float UStatsComponent::ApplyDamage(float Damage, float Penetration)
{
    if (bDmgImmune || GetStat(EStat::Health) <= 0.0f || GetStat(EStat::HealthMax) <= 0.0f) return 0.0f;

	float prevPercent = GetStat(EStat::Health) / GetStat(EStat::HealthMax);

    // Apply health damage
	Damage = FMath::Clamp(Damage, 0.0f, FLT_MAX);
	Penetration = FMath::Clamp(Penetration, 0.0f, 1.0f);
	
	float defense = GetStat(EStat::Defense);
	float effectiveDefense = defense > 0.0f ? defense - (defense * Penetration) : 0.0f; // Apply penetration

	Damage *= (100.0f / (100.0f + effectiveDefense)); //Diminsihing returns formula
	stats[EStat::Health] = FMath::Clamp(stats[EStat::Health] - Damage, 0.0f, stats[EStat::HealthMax]);

	float newPercent = GetStat(EStat::Health) / GetStat(EStat::HealthMax);

	// Events
    OnHealthUpdate.Broadcast(newPercent, prevPercent);
    if (stats[EStat::Health] <= 0.f) OnZeroHealth.Broadcast();
	return Damage;
}

/* -------------------------- Health Addition -------------------------- */
void UStatsComponent::AddToCurrentHealth(float Percent)
{
	float health = GetStat(EStat::Health);
	float maxHealth = GetStat(EStat::HealthMax);

	if (health < 0.0f || maxHealth <= 0.0f) return;

	float prevPercent = health / maxHealth;

	Percent = FMath::Clamp(Percent, 0, 1);
	float newHealth = health + (maxHealth * Percent);
	newHealth = FMath::Clamp(newHealth, 0.0f, maxHealth);
	stats[EStat::Health] = newHealth;

	float newPercent = health / maxHealth;

	OnHealthUpdate.Broadcast(newPercent, prevPercent);
}