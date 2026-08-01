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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 0.0f, ToolTip = "For warping. Prevents the enemy from warping directly ontop of their target. 0 means don't warp transationally"))
	float warpOffset = 0.0f;
};