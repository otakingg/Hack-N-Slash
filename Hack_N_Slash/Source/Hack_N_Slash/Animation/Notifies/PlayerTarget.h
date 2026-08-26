#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "../../Enums/ETargetingStyle.h"
#include "PlayerTarget.generated.h"

/**
 * 
 */
UCLASS()
class HACK_N_SLASH_API UPlayerTarget : public UAnimNotify
{
	GENERATED_BODY()

protected:
    UPROPERTY(EditAnywhere, Category = "Data")
    bool bDebug = false;

	UPROPERTY(EditAnywhere, Category = "Data")
	bool bSnapToInputDirectionIfNoTarget = true;

	UPROPERTY(EditAnywhere, Category = "Data|Target", meta = (ToolTip = "Targetting style when locked on"))
	ETargetingStyle targetingStyle = ETargetingStyle::None;

	UPROPERTY(EditAnywhere, Category = "Data|Target", meta = (ToolTip = "Maximum height difference a target can be to be soft-targettable"))
	float softHeightCeiling = 150.0f;

	UPROPERTY(EditAnywhere, Category = "Data|Target", meta = (ClampMin = "0.0"), meta = (ToolTip = "Maximum radius a target can be to be soft-targettable"))
	float softRadius = 300.0f;

	UPROPERTY(EditAnywhere, Category = "Data|Target", meta = (ClampMin = "0.0"), meta = (ToolTip = "Maximum radius a target can be to be free-flowable"))
	float freeFlowRadius = 1200.0f;
	
    UPROPERTY(EditAnywhere, Category = "Data|Warp", meta = (ToolTip = "How far away from the enemy the character wants to warp to"))
    float warpLocOffset = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Data|Warp", meta = (ToolTip = "In regards to the desired warp rotation"))
	bool bIgnorePitch = true;

	UPROPERTY(EditAnywhere, Category = "Data|Warp", meta = (ToolTip = "In regards to the desired warp rotation"))
	bool bIgnoreRoll = true;

	UPROPERTY(EditAnywhere, Category = "Data|Warp", meta = (ToolTip = "In regards to the desired warp rotation"))
	bool bIgnoreYaw = false;

	UPROPERTY(EditAnywhere, Category = "Data|Warp", meta = (ToolTip = "In regards to the desired warp location"))
	bool bIgnoreTranslation = false;

	UPROPERTY(EditAnywhere, Category = "Data|Warp", meta = (ToolTip = "In regards to the desired warp location. 0 means no limit. Only hard lock on because targetting radius ends up being the limit for soft lock on"))
	float maxWarpTranslDistLockOn = 500.0f;

public:
    UPlayerTarget();
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
