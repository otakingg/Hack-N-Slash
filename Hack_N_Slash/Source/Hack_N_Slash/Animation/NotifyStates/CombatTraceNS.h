// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "../Structs/FAtkHitData.h"
#include "../Structs/FSocketTrace.h"
#include "CombatTraceNS.generated.h"

UENUM(BlueprintType)
enum class ETraceTypeNS : uint8
{
    Distance,
    Socket
};

/**
 * 
 */
UCLASS()
class HACK_N_SLASH_API UCombatTraceNS : public UAnimNotifyState
{
	GENERATED_BODY()

private:
	class UCombatTraceComponent* traceComp;

public:
	UPROPERTY(EditAnywhere, Category = "Trace")
	ETraceTypeNS traceType = ETraceTypeNS::Distance;

	UPROPERTY(EditAnywhere, Category = "Trace")
	FAtkHitData hitData;

	UPROPERTY(EditAnywhere, Category = "Trace")
	float traceRadius;

	UPROPERTY(EditAnywhere, Category = "Trace|Distance")
	FVector traceDistance;

	UPROPERTY(EditAnywhere, Category = "Trace|Distance", meta = (ToolTip = "This will be added to the start location of the owner"))
	FVector traceOffset;

	UPROPERTY(EditAnywhere, Category = "Trace|Socket")
	TArray<FSocketTrace> sockets;

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
