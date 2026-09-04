// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "StateMachine.generated.h"

/**
 * Sends notify events to the State Machine Component
 */
UCLASS()
class HACK_N_SLASH_API UStateMachine : public UAnimNotify
{
	GENERATED_BODY()

protected:
    UPROPERTY(EditAnywhere, Category = "State Machine")
    TArray<FGameplayTag> notifyTags;

public:
    UStateMachine();
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
};