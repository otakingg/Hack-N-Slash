// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "PauseMont.generated.h"

/**
 * Pauses the current montage
 */
UCLASS()
class HACK_N_SLASH_API UPauseMont : public UAnimNotify
{
	GENERATED_BODY()

public:
    UPauseMont();
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
