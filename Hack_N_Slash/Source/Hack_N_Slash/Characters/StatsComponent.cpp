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

	if (HasStat(EStat::StaggerMax))
	{
		stats.FindOrAdd(EStat::Stagger) = FMath::Clamp(GetStat(EStat::Stagger), 0.f, stats[EStat::StaggerMax]);
		stats[EStat::Stagger] = stats[EStat::StaggerMax];
	}
	bCanRegenStagger = false;
}

void UStatsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (HasStat(EStat::Stagger) && HasStat(EStat::StaggerMax))
    {
		RegenStat(DeltaTime, stats[EStat::Stagger], stats[EStat::StaggerMax], bStaggerBroken? stats[EStat::StaggerRegen] * 0.75f : stats[EStat::StaggerRegen], bCanRegenStagger);
	}
	else if (bDebug && GEngine) {GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("Stats Comp: Tick: Stagger and/or Max Stagger not found"));}
}

bool UStatsComponent::HasStat(EStat Stat) const { return stats.Contains(Stat); }

float UStatsComponent::GetStatPercentage(EStat Current, EStat Max) const
{
	float current {GetStat(Current)};
	float max {GetStat(Max)};
	return max > 0.0f ? current / max : 0.0f;
}

float UStatsComponent::GetStat(EStat Stat) const
{
	if (const float* val = stats.Find(Stat)) return *val;
	return 0.f;
}

void UStatsComponent::RegenStat(float DeltaTime, float& Val, float Max, float Rate, bool bCanRegen)
{
	if (!bCanRegen || Val >= Max) {return;}

	Val = UKismetMathLibrary::FInterpTo_Constant(Val, Max, DeltaTime, Rate);

	if (GetStat(EStat::Stagger) >= GetStat(EStat::StaggerMax)) {bStaggerBroken = false; bCanRegenStagger = false;}
}

void UStatsComponent::ApplyDamage(const FAtkHitData& hitData)
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

	// Stagger Damage
	if (HasStat(EStat::Stagger) && HasStat(EStat::StaggerMax)) {HandleStaggerDamage(hitData);}

    // DEATH CHECK
    if (stats[EStat::Health] <= 0.f) OnZeroHealthUpdateDelegate.Broadcast();
}

void UStatsComponent::HandleStaggerDamage(const FAtkHitData& hitData)
{
    // APPLY STAGGER if it hasn't been broken yet
	if (!bStaggerBroken)
	{
		float dmgStagger {hitData.baseDmgStagger}; //Damage done to stagger meter
		dmgStagger *= (100.0f / (100.0f + GetStat(EStat::StaggerResist))); //Diminishing returns formula
		stats[EStat::Stagger] = FMath::Clamp(stats[EStat::Stagger] - dmgStagger, 0.f, stats[EStat::StaggerMax]);
	}

	if (stats[EStat::Stagger] <= 0.f)
	{
		bStaggerBroken = true;
		bCanRegenStagger = false;
		FLatentActionInfo staggerRegenInfo {0, 103, TEXT("EnableRegenStagger"), this};
		UKismetSystemLibrary::RetriggerableDelay(GetWorld(), stats[EStat::StaggerRegenDelay], staggerRegenInfo);
	}
}

void UStatsComponent::EnableRegenStagger() {bCanRegenStagger = true;}