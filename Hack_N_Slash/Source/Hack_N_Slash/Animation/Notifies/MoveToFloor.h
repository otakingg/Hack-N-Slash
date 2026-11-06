#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameFramework/RootMotionSource.h"
#include "MoveToFloor.generated.h"

/**
 * 
 */
UCLASS()
class HACK_N_SLASH_API UMoveToFloor : public UAnimNotify
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditAnywhere, Category = "Floor")
	bool bDebug = false;

	UPROPERTY(EditAnywhere, Category = "Floor|Trace", meta = (ClampMin = 0))
	float traceDistance = 3000.0f;

	UPROPERTY(EditAnywhere, Category = "Floor|Movement")
	ERootMotionFinishVelocityMode velocityOnFinishMode = ERootMotionFinishVelocityMode::SetVelocity;

	UPROPERTY(EditAnywhere, Category = "Floor|Movement", meta = (EditCondition = "VelocityOnFinishMode == ERootMotionFinishVelocityMode::SetVelocity", EditConditionHides))
	FVector velocityOnFinish = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Floor|Movement", meta = (EditCondition = "VelocityOnFinishMode == ERootMotionFinishVelocityMode::ClampVelocity", EditConditionHides, ClampMin = 0))
	float clampVelocityOnFinish = 0.0f;

    UPROPERTY(EditAnywhere, Category = "Floor|Movement", meta = (ClampMin = 0))
    float duration = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Floor|Movement")
	bool bRestrictSpeedToExpected = true;
	
public:
	UMoveToFloor();
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
