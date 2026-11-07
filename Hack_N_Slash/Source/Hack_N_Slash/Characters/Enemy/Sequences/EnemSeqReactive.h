#pragma once

#include "CoreMinimal.h"
#include "Characters/Enemy/Sequences/EnemySequence.h"
#include "EnemSeqReactive.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UEnemSeqReactive : public UEnemySequence
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sequence|Chance", meta = (ClampMin = 0, ClampMax = 1, ToolTip = "How likely is this sequence to execute once chosen. 0 = Never, 1 = Always"))
	float chance = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sequence|Chance", meta = (ClampMin = 0, ClampMax = 1, ToolTip = "How much each recent hit increases execution chance"))
	float recentHitsModifier = 0.05f;

	UFUNCTION(BlueprintPure, Category = "Sequence")
	bool IsFacingTarget(AActor* Target, float Tolerance = 0.7f) const;

public:
	UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "Sequence")
	bool CanExecute(const FAtkHitData& HitData) const;
    virtual bool CanExecute_Implementation(const FAtkHitData& HitData) const;

	UFUNCTION(BlueprintPure, Category = "Sequence")
	float GetScore() const { return baseScore; }

	bool RollChance() const; // Rolls the chance to determine if this sequence should execute
};