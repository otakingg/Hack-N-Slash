#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "../../Enums/ETargetingStyle.h"
#include "PlayerTargetNS.generated.h"

/**
 * Tries to get a soft target for the player based on paramters
 * Updates warp information in the Locomotion Component
 */
UCLASS()
class HACK_N_SLASH_API UPlayerTargetNS : public UAnimNotifyState
{
	GENERATED_BODY()

protected:
    UPROPERTY(EditAnywhere, Category = "Data")
    bool bDebug = false;

	UPROPERTY(EditAnywhere, Category = "Data")
	bool bSnapToInputDirectionIfNoTarget = true;

	UPROPERTY(EditAnywhere, Category = "Data|Target", meta = (ToolTip = "Targetting style. Irrelevent when locked off"))
	ETargetingStyle targetingStyle = ETargetingStyle::None;

	UPROPERTY(EditAnywhere, Category = "Data|Target", meta = (ClampMin = 0, ToolTip = "Maximum height difference a target can be to be soft-targettable"))
	float softHeightCeiling = 150.0f;

	UPROPERTY(EditAnywhere, Category = "Data|Target", meta = (ClampMin = 0), meta = (ToolTip = "Maximum radius a target can be to be soft-targettable"))
	float softRadius = 300.0f;

	UPROPERTY(EditAnywhere, Category = "Data|Target", meta = (ClampMin = 0), meta = (ToolTip = "Maximum radius a target can be to be free-flowable", EditCondition = "TargetingStyle == ETargetingStyle::AlignMove || TargetingStyle == ETargetingStyle::AlignMoveOrCam || TargetingStyle == ETargetingStyle::AlignMoveOrDist", EditConditionHides))
	float freeFlowRadius = 1200.0f;
	
    UPROPERTY(EditAnywhere, Category = "Data|Warp", meta = (ToolTip = "Useful for avoiding warping directly on top of your target"))
    float offsetDistance = 150.0f;

    UPROPERTY(EditAnywhere, Category = "Data|Warp", meta = (ToolTip = "Useful for positioning yourself above/below your target"))
    float offsetVertical = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Data|Warp", meta = (ToolTip = "In regards to the desired warp rotation"))
	bool bIgnorePitch = true;

	UPROPERTY(EditAnywhere, Category = "Data|Warp", meta = (ToolTip = "In regards to the desired warp rotation"))
	bool bIgnoreRoll = true;

	UPROPERTY(EditAnywhere, Category = "Data|Warp", meta = (ToolTip = "In regards to the desired warp rotation"))
	bool bIgnoreYaw = false;

	UPROPERTY(EditAnywhere, Category = "Data|Warp", meta = (ToolTip = "In regards to the desired warp location"))
	bool bIgnoreTranslation = false;

	UPROPERTY(EditAnywhere, Category = "Data|Warp", meta = (ToolTip = "Only neccessary for lock-on because 'free flow radius' is the limit for lock-off"))
	bool bLimitTranslDistLockOn = true;

public:
	UPlayerTargetNS();
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
};
