#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GameFramework/RootMotionSource.h"
#include "RMS_MoveToDyn.generated.h"

/**
 * Applies a Root Motion Source_MoveToDynamic to the owner
 */
UCLASS()
class HACK_N_SLASH_API URMS_MoveToDyn : public UAnimNotifyState
{
	GENERATED_BODY()

protected:
    UPROPERTY(EditAnywhere, Category = "Root Motion")
    bool bDebug = false;

	UPROPERTY(EditAnywhere, Category = "Root Motion")
	ERootMotionFinishVelocityMode velocityOnFinishMode = ERootMotionFinishVelocityMode::SetVelocity;

	UPROPERTY(EditAnywhere, Category = "Root Motion", meta = (EditCondition = "VelocityOnFinishMode == ERootMotionFinishVelocityMode::SetVelocity", EditConditionHides))
	FVector velocityOnFinish = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Root Motion", meta = (EditCondition = "VelocityOnFinishMode == ERootMotionFinishVelocityMode::ClampVelocity", EditConditionHides, ClampMin = 0))
	float clampVelocityOnFinish = 0.0f;

    UPROPERTY(EditAnywhere, Category = "Root Motion", meta = (ClampMin = 0, Tooltip = "Set this equal to duration of this anim notify state"))
    float duration = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Root Motion")
	bool bRestrictSpeedToExpected = true;

public:
	URMS_MoveToDyn();
    virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
};