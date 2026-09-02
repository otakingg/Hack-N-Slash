#pragma once

#include "CoreMinimal.h"
#include "FSocketTrace.generated.h"

// Used by the "Combat Trace Component" to define a trace between 2 sockets on a skeletal mesh for hit detection
USTRUCT(BlueprintType)
struct HACK_N_SLASH_API FSocketTrace
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, meta = (ToolTip = "Name of the socket where a trace should begin"))
	FName socketStart;

	UPROPERTY(EditAnywhere, meta = (ToolTip = "Name of the socket where a trace should end"))
	FName socketEnd;

	UPROPERTY(EditAnywhere, meta = (ToolTip = "Will be added to the trace length"))
	float extraLength = 0.0f;
};