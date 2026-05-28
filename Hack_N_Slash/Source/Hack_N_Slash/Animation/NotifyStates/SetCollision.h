// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "SetCollision.generated.h"

/**
 * 
 */
UCLASS()
class HACK_N_SLASH_API USetCollision : public UAnimNotifyState
{
	GENERATED_BODY()

private:
	// Caution: This will be shared by all actors, so if multiple actors use this notify state at once, then the responses can get muddied. Need a fix around this
	TMap<ECollisionChannel, ECollisionResponse> responses;

protected:
	UPROPERTY(EditAnywhere, Category = "Collision")
	TArray<TEnumAsByte<ECollisionChannel>> channlesToBlock;
	
	UPROPERTY(EditAnywhere, Category = "Collision")
	TArray<TEnumAsByte<ECollisionChannel>> channlesToIgnore;

	UPROPERTY(EditAnywhere, Category = "Collision")
	TArray<TEnumAsByte<ECollisionChannel>> channlesToOverlap;
	
public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
};