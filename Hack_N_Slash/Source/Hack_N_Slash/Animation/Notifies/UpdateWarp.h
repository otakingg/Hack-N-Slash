// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "UpdateWarp.generated.h"

/**
 * 
 */
UCLASS()
class HACK_N_SLASH_API UUpdateWarp : public UAnimNotify
{
	GENERATED_BODY()

protected:
    UPROPERTY(EditAnywhere, Category = "Root Motion")
    bool bDebug = false;
	
    UPROPERTY(EditAnywhere, Category = "Root Motion")
    float offset = 0.0f;

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
