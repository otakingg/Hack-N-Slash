#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "EnemyTarget.generated.h"

/**
 * Updates warp info in the Locomotion Component
 */
UCLASS()
class HACK_N_SLASH_API UEnemyTarget : public UAnimNotify
{
	GENERATED_BODY()

protected:
    UPROPERTY(EditAnywhere, Category = "Data")
    bool bDebug = false;

	UPROPERTY(EditAnywhere, Category = "Data|Warp", meta = (ToolTip = "In regards to the desired warp rotation"))
	bool bIgnorePitch = true;

	UPROPERTY(EditAnywhere, Category = "Data|Warp", meta = (ToolTip = "In regards to the desired warp rotation"))
	bool bIgnoreRoll = true;

	UPROPERTY(EditAnywhere, Category = "Data|Warp", meta = (ToolTip = "In regards to the desired warp rotation"))
	bool bIgnoreYaw = false;

	UPROPERTY(EditAnywhere, Category = "Data|Warp", meta = (ToolTip = "In regards to the desired warp translation"))
	bool bIgnoreTranslation = false;
	
    UPROPERTY(EditAnywhere, Category = "Data|Warp", meta = (ToolTip = "Useful for avoiding warping directly on top of your target"))
    float offsetDistance = 100.0f;

    UPROPERTY(EditAnywhere, Category = "Data|Warp", meta = (ToolTip = "Useful for positioning yourself above/below your target"))
    float offsetVertical = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Data|Warp", meta = (ClampMin = 0, ToolTip = "How far is the enemy allowed to translate for this attack? 0 means no limit"))
	float maxTranslDist = 300.0f;

	UPROPERTY(EditAnywhere, Category = "Data|Target", meta = (ClampMin = 0, ToolTip = "Maximum height difference a target can be to be a valid warp target. 0 means no limit"))
	float heightCeiling = 150.0f;

	UPROPERTY(EditAnywhere, Category = "Data|Target", meta = (ClampMin = 0, ClampMax = 0.5f), meta = (ToolTip = "Target the position the target will be in this seconds. 0 means target their current position instead"))
	float predictionTime = 0.1f;

public:
    UEnemyTarget();
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};