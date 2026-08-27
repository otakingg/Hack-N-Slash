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
	UPROPERTY(EditAnywhere, Category = "Rotate", meta = (ClampMin = 0.0f, Tooltip = "0 means speed is calculated based on distance to target, and desired time"))
	float rotationSpeed = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Rotate", meta = (ClampMin = 0.01f, Tooltip = "Set this equal to the length of the notify state. Only used if rotaion speed = 0"))
	float rotationDuration = 0.1f;

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
