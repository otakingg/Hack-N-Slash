#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "RotateToTarget.generated.h"

/**
 * 
 */
UCLASS()
class HACK_N_SLASH_API URotateToTarget : public UAnimNotifyState
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditAnywhere, Category = "Rotate")
	float rotationSpeed = 50.0f;

	UPROPERTY(EditAnywhere, Category = "Rotate", meta = (ToolTip = "In regards to the desired warp rotation"))
	bool bIgnorePitch = true;

	UPROPERTY(EditAnywhere, Category = "Rotate", meta = (ToolTip = "In regards to the desired warp rotation"))
	bool bIgnoreRoll = true;

	UPROPERTY(EditAnywhere, Category = "Rotate", meta = (ToolTip = "In regards to the desired warp rotation"))
	bool bIgnoreYaw = false;

public:
	URotateToTarget();
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
};
