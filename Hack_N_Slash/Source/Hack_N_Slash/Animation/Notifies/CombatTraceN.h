// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "../Structs/FAtkHitData.h"
#include "../Structs/FSocketTrace.h"
#include "CombatTraceN.generated.h"

UENUM(BlueprintType)
enum class ETraceTypeN : uint8
{
    Distance,
    Socket
};

/**
 * 
 */
UCLASS()
class HACK_N_SLASH_API UCombatTraceN : public UAnimNotify
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Trace")
	ETraceTypeN traceType = ETraceTypeN::Distance;

	UPROPERTY(EditAnywhere, Category = "Trace")
	FAtkHitData hitData;

	UPROPERTY(EditAnywhere, Category = "Trace")
	float traceRadius;

	UPROPERTY(EditAnywhere, Category = "Trace|Distance")
	float traceDistance;

	UPROPERTY(EditAnywhere, Category = "Trace|Distance", meta = (ToolTip = "This will be added to the start location of the owner"))
	FVector traceOffset;

	UPROPERTY(EditAnywhere, Category = "Trace|Socket")
	TArray<FSocketTrace> sockets;
	
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
};
