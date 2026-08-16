#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "FEnemyAtkData.generated.h"

USTRUCT(BlueprintType)
struct FEnemyAtkData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimMontage* montage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName montageSection = NAME_None;
};