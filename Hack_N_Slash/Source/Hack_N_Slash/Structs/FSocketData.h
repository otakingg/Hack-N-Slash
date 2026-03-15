#pragma once

#include "CoreMinimal.h"
#include "FSocketData.generated.h"

USTRUCT(BlueprintType)
struct HACK_N_SLASH_API FSocketData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, meta = (ToolTip = "Name of the socket where a trace should begin"))
	FName socketStart;

	UPROPERTY(EditAnywhere, meta = (ToolTip = "Name of the socket where a trace should end"))
	FName socketEnd;
};