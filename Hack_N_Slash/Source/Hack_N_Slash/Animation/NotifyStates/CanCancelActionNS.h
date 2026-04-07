// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "CanCancelActionNS.generated.h"

/**
 * 
 */
UCLASS()
class HACK_N_SLASH_API UCanCancelActionNS : public UAnimNotifyState
{
	GENERATED_BODY()
	
private:
	class UPlayerCombatCancelComponent* playerCombatCancelComp;

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
