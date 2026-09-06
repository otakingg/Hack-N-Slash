// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "LaunchChar.generated.h"

/**
 * Launches the owner
 */
UCLASS()
class HACK_N_SLASH_API ULaunchChar : public UAnimNotify
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditAnywhere, Category = "Launch")
	FVector velocity = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Launch")
	bool bOverrideXY = false;

	UPROPERTY(EditAnywhere, Category = "Launch")
	bool bOverrideZ = false;

	UPROPERTY(EditAnywhere, Category = "Launch", meta = (ClampMin = 0))
	float timeToStop = 0.0f;
	
public:
	ULaunchChar();
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
