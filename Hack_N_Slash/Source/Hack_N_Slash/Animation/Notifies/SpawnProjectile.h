// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "SpawnProjectile.generated.h"

class AProjectileBase;

/**
 * 
 */
UCLASS()
class HACK_N_SLASH_API USpawnProjectile : public UAnimNotify
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Custom")
	bool bDebug = false;

	UPROPERTY(EditAnywhere, Category = "Custom")
	bool bIgnoreSelf = true;

	UPROPERTY(EditAnywhere, Category = "Custom")
	TSubclassOf<AProjectileBase> projectileClass;

	UPROPERTY(EditAnywhere, Category = "Custom")
	FVector spawnOffset;

	UPROPERTY(EditAnywhere, Category = "Custom")
	FName spawnSocketName;
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
