// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AOE_Base.h"
#include "../../Structs/FAtkHitData.h"
#include "AOE_Damage_Base.generated.h"

class UNiagaraSystem;
/**
 * 
 */
UCLASS()
class HACK_N_SLASH_API AAOE_Damage_Base : public AAOE_Base
{
	GENERATED_BODY()

private:
	float CalculateDamage() const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	FVector impactScale {1.0f, 1.0f, 1.0f};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	USoundBase* impactSFX = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	UNiagaraSystem* impactVFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
	AActor* target = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	FAtkHitData hitData;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float damage = 0.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float penetration = 0.0f;

	UFUNCTION(BlueprintCallable, Category = "Damage")
	void HandleDamage(AActor* HitActor);
};
