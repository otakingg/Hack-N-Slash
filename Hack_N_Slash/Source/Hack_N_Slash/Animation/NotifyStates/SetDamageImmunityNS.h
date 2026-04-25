// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "SetDamageImmunityNS.generated.h"

/**
 * 
 */
UCLASS()
class HACK_N_SLASH_API USetDamageImmunityNS : public UAnimNotifyState
{
	GENERATED_BODY()

private:
	class UStatsComponent* statsComp = nullptr;

public:
	UPROPERTY(EditAnywhere, Category = "Immunity")
	bool startImmunity = true;
	
	UPROPERTY(EditAnywhere, Category = "Immunity")
	bool endImmunity = false;

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};