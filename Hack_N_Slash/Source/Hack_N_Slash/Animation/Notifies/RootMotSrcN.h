// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "RootMotSrcN.generated.h"

UENUM(BlueprintType)
enum class ERootMotionType : uint8
{
    None,
    Constant,
	Jump,
	MoveToDynamic,
	MoveTo,
	Radial
};

/**
 * 
 */
UCLASS()
class HACK_N_SLASH_API URootMotSrcN : public UAnimNotify
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Root Motion Source")
	bool bDebug = false;

	UPROPERTY(EditAnywhere, Category = "Root Motion Source")
	ERootMotionType sourceType = ERootMotionType::None;

	UPROPERTY(EditAnywhere, Category = "Root Motion Source|Constant Force")
	FVector force = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Root Motion Source|Constant Force")
	float constantDuration = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Root Motion Source|Jump Force")
	float distance = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Root Motion Source|Jump Force")
	float height = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Root Motion Source|Move To")
	float offset = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Root Motion Source|Radial Force")
	float radius = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Root Motion Source|Radial Force")
	float strength = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Root Motion Source|Radial Force")
	FVector location = FVector::ZeroVector;

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;	
};