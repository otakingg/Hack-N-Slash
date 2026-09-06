#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "RotateToTarget.generated.h"

/**
 * Rotates the owner to the warp rotation stored in the Locomotion Component
 */
UCLASS()
class HACK_N_SLASH_API URotateToTarget : public UAnimNotifyState
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditAnywhere, Category = "Rotate", meta = (ClampMin = 0, Tooltip = "0 means speed is calculated based on distance to target, and desired duration"))
	float speed = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Rotate", meta = (ClampMin = 0.01, EditCondition = "Speed == 0", EditConditionHides, Tooltip = "Set = to notify length"))
	float duration = 0.1f;

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
