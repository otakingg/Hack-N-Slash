// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "StateMachineN.generated.h"

/**
 * 
 */
UCLASS()
class HACK_N_SLASH_API UStateMachineN : public UAnimNotify
{
	GENERATED_BODY()

protected:
    UPROPERTY(EditAnywhere)
    FName notifyName;

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
};
