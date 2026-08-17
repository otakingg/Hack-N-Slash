#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "PlayerTargetNS.generated.h"

/**
 * 
 */
UCLASS()
class HACK_N_SLASH_API UPlayerTargetNS : public UAnimNotifyState
{
	GENERATED_BODY()

protected:
    UPROPERTY(EditAnywhere, Category = "Data")
    bool bDebug = false;

	UPROPERTY(EditAnywhere, Category = "Data", meta = (ToolTip = "Is this atack free-flowable"))
	bool bFreeFlow = false;

	UPROPERTY(EditAnywhere, Category = "Data")
	bool bSnapToInputDirectionIfNoTarget = true;
	
    UPROPERTY(EditAnywhere, Category = "Data|Warp", meta = (ToolTip = "How far away from the enemy the character wants to warp to"))
    float warpLocOffset = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Data|Warp", meta = (ToolTip = "In regards to the desired warp rotation"))
	bool bIgnorePitch = true;

	UPROPERTY(EditAnywhere, Category = "Data|Warp", meta = (ToolTip = "In regards to the desired warp rotation"))
	bool bIgnoreRoll = true;

	UPROPERTY(EditAnywhere, Category = "Data|Warp", meta = (ToolTip = "In regards to the desired warp rotation"))
	bool bIgnoreYaw = false;

	UPROPERTY(EditAnywhere, Category = "Data|Target", meta = (ToolTip = "Maximum height difference a target can be to be soft-targettable"))
	float softHeightCeiling = 150.0f;

	UPROPERTY(EditAnywhere, Category = "Data|Target", meta = (ClampMin = "0.0"), meta = (ToolTip = "Maximum radius a target can be to be soft-targettable"))
	float softRadius = 750.0f;

	UPROPERTY(EditAnywhere, Category = "Data|Target", meta = (ClampMin = "0.0"), meta = (ToolTip = "Maximum radius a target can be to be free-flowable"))
	float freeFlowRadius = 1500.0f;

public:
	UPlayerTargetNS();
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
};
