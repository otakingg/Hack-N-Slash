// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../../Enums/EStat.h"
#include "StatsComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_OneParam(FOnHealthPercentUpdateSignature, UStatsComponent, OnHealthUpdateDelegate, float, newPercent);
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE(FOnZeroHealthUpdateSignature, UStatsComponent, OnZeroHealthUpdateDelegate);

struct FAtkHitData;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HACK_N_SLASH_API UStatsComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	void RegenStat(float, float&, float, float, bool);
	bool HasStat(EStat) const;

protected:
	UPROPERTY(EditAnywhere)
	bool bDebug {false};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<EStat, float> stats;

	virtual void BeginPlay() override;

public:
	UPROPERTY(BlueprintAssignable)
	FOnHealthPercentUpdateSignature OnHealthUpdateDelegate;

	UPROPERTY(BlueprintAssignable)
	FOnZeroHealthUpdateSignature OnZeroHealthUpdateDelegate;

	UStatsComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintPure)
	float GetStatPercentage(EStat Current, EStat Max) const;

	UFUNCTION(BlueprintPure)
	float GetStat(EStat Stat) const;

	void RecieveHit(const FAtkHitData&);
};