// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "UpdateMW.generated.h"

/**
 * 
 */
UCLASS()
class HACK_N_SLASH_API UUpdateMW : public UAnimNotifyState
{
	GENERATED_BODY()

protected:
    UPROPERTY(EditAnywhere, Category = "Root Motion")
    bool bDebug = false;
	
    UPROPERTY(EditAnywhere, Category = "Root Motion")
    float offset = 200.0f;
    
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
};