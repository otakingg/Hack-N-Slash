// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../../Enums/EStat.h"
#include "StatsComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthPercentUpdate, float, NewPercent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnZeroHealthUpdate);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HACK_N_SLASH_API UStatsComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	void RegenStat(float, float&, float, float, bool);
	bool HasStat(EStat) const;

protected:
	UPROPERTY(EditAnywhere)
	bool bDebug = false;

	UPROPERTY(EditAnywhere)
	bool bDmgImmune = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<EStat, float> stats;

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
public:
	UPROPERTY(BlueprintAssignable)
	FOnHealthPercentUpdate OnHealthUpdate;

	UPROPERTY(BlueprintAssignable)
	FOnZeroHealthUpdate OnZeroHealthUpdate;

	UStatsComponent();

	UFUNCTION(BlueprintPure)
	float GetStatPercentage(EStat Current, EStat Max) const;

	UFUNCTION(BlueprintPure)
	float GetStat(EStat Stat) const;

	void SetStat(EStat Stat, float Value);

	bool IsAlive() const { return GetStat(EStat::Health) > 0.0f; }

	float ApplyDamage(float HealthDmg = 0.0f, float Penetration = 0.0f);

	void SetDmgImmunity(bool bImmune) { bDmgImmune = bImmune; }
};