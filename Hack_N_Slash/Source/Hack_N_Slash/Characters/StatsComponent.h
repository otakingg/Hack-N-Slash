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
	void RegenStat(float&, float, float, bool);

protected:
	UPROPERTY(EditAnywhere)
	bool bDebugMode {false};

	UPROPERTY(VisibleAnywhere, Category = Stagger)
	bool bStaggerBroken {false};
	
	UPROPERTY(VisibleAnywhere, Category = Stagger)
	bool bCanRegenStagger {false};

	virtual void BeginPlay() override;

	UFUNCTION()
	void EnableRegenStagger();

public:
	UPROPERTY(BlueprintAssignable)
	FOnHealthPercentUpdateSignature OnHealthUpdateDelegate;

	UPROPERTY(BlueprintAssignable)
	FOnZeroHealthUpdateSignature OnZeroHealthUpdateDelegate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<EStat, float> stats;

	UStatsComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void ApplyDamage(const FAtkHitData&);

	UFUNCTION(BlueprintPure)
	float GetStatPercentage(EStat current, EStat max) const;
};