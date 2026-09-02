#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "FEnemyAtkData.generated.h"

// Defines enemies' attack data
USTRUCT(BlueprintType)
struct FEnemyAtkData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimMontage* montage = nullptr; // The montage to play

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName montageSection = NAME_None; // The montage section to jump to
};