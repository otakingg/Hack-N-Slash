// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "SetDamageImmunity.generated.h"

/**
 * Modifies wether the owner is immune to damage or not
 */
UCLASS()
class HACK_N_SLASH_API USetDamageImmunity : public UAnimNotifyState
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Immunity")
	bool startImmunity = true;
	
	UPROPERTY(EditAnywhere, Category = "Immunity")
	bool endImmunity = false;
	
public:
	USetDamageImmunity();
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};