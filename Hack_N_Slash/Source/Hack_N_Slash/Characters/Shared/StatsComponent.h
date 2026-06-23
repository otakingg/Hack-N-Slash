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

	UFUNCTION(BlueprintPure, Category = "Stats")
	bool HasStat(EStat Stat) const;

	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetStat(EStat Stat) const;

	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetStatPercentage(EStat Current, EStat Max) const;

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetStat(EStat Stat, float Value);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetDmgImmunity(bool bImmune) { bDmgImmune = bImmune; }

	UFUNCTION(BlueprintPure, Category = "Stats")
	bool IsAlive() const { return GetStat(EStat::Health) > 0.0f; }

	float ApplyDamage(float HealthDmg = 0.0f, float Penetration = 0.0f);
};