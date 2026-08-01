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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sequence|Score", meta = (ToolTip = "How does aggro affect this sequence. If left empty, won't affect score"))
	UCurveFloat* aggroCurve = nullptr;	

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sequence|Score", meta = (ToolTip = "How does this enemy want to perform this sequence based on target distance. If left empty, won't affect score"))
	UCurveFloat* distanceCurve = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sequence|Score", meta = (ToolTip = "How does this enemy want to perform this sequence based on how recent their last atk was. If left empty, won't affect score"))
	UCurveFloat* timeSinceLastAtkCurve = nullptr;

	UFUNCTION(BlueprintPure, Category = "Sequence")
	float GetAggroMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Sequence")
	float GetAtkTimeMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Sequence")
	float GetDistanceMultiplier() const;

public:
	UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "Sequence")
	bool CanExecute() const;
    virtual bool CanExecute_Implementation() const;
	
	UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "Sequence")
	float GetScore() const;
	virtual float GetScore_Implementation() const;
};