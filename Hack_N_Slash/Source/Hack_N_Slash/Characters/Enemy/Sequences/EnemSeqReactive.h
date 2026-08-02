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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sequence", meta = (ClampMin = 0, ClampMax = 1, ToolTip = "Probability of performing this sequence when it's chosen. Base score is still used to choose a sequence"))
	float reactionChance = 0.5f;

	UFUNCTION(BlueprintPure, Category = "Sequence")
	bool IsFacingTarget(AActor* Target, float Tolerance = 0.5f) const;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sequence")
	bool bPreHit = true;

	UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "Sequence")
	bool CanExecute(const FAtkHitData& HitData) const;
    virtual bool CanExecute_Implementation(const FAtkHitData& HitData) const;

	UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "Sequence")
	float GetScore(const FAtkHitData& HitData) const;
	virtual float GetScore_Implementation(const FAtkHitData& HitData) const { return baseScore; }

	float GetReactionChance() const { return reactionChance; }
};