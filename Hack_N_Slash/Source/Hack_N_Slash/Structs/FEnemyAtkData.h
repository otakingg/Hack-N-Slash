#pragma once

#include "CoreMinimal.h"
#include "FEnemyAtkData.generated.h"

USTRUCT(BlueprintType)
struct FEnemyAtkData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimMontage* montage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 200.0f, ToolTip = "For warping. Prevents the enemy from warping directly ontop of their target"))
	float warpOffset = 200.0f;
};