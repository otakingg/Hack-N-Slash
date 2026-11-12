#pragma once

#include "CoreMinimal.h"
#include "Characters/Enemy/Sequences/EnemySequence.h"
#include "EnemSeqProactive.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UEnemSeqProactive : public UEnemySequence
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sequence|Score", meta = (ToolTip = "How does this enemy want to perform this sequence based on target distance. If left empty, won't affect score"))
	UCurveFloat* distanceCurve = nullptr; // Distance score multiplier

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sequence|Score", meta = (ToolTip = "How does frequency affect this sequence. If left empty, won't affect score"))
	UCurveFloat* stalenessCurve = nullptr; // Sequence staleness multiplier

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sequence|Score", meta = (ToolTip = "How does this enemy want to perform this sequence based on how recent their last atk was. If left empty, won't affect score"))
	UCurveFloat* timeSinceLastAtkCurve = nullptr; // Time isnce last attack score multiplier

	UFUNCTION(BlueprintPure, Category = "Sequence")
	float GetAtkTimeMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Sequence")
	float GetDistanceMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Sequence")
	float GetStalenessMultiplier() const;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sequence|Cooldown", meta = (ToolTip = "Should this sequence always happen off cooldown?"))
	bool bForceOffCooldown = false;
	
	UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "Sequence")
	bool CanExecute() const;
    virtual bool CanExecute_Implementation() const;
	
	UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "Sequence")
	float GetScore() const;
	virtual float GetScore_Implementation() const;
};