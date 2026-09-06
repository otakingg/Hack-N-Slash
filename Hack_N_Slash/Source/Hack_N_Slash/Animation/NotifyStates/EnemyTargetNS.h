#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "EnemyTargetNS.generated.h"

/**
 * Updates warp info in the Locomotion Component
 */
UCLASS()
class HACK_N_SLASH_API UEnemyTargetNS : public UAnimNotifyState
{
	GENERATED_BODY()

protected:
    UPROPERTY(EditAnywhere, Category = "Data")
    bool bDebug = false;
	
    UPROPERTY(EditAnywhere, Category = "Data|Warp", meta = (ClampMin = 0, ToolTip = "Will warp this from the target. Useful for avoiding warping directly on top of your target"))
    float offset = 150.0f;

	UPROPERTY(EditAnywhere, Category = "Data|Warp", meta = (ToolTip = "In regards to the desired warp rotation"))
	bool bIgnorePitch = true;

	UPROPERTY(EditAnywhere, Category = "Data|Warp", meta = (ToolTip = "In regards to the desired warp rotation"))
	bool bIgnoreRoll = true;

	UPROPERTY(EditAnywhere, Category = "Data|Warp", meta = (ToolTip = "In regards to the desired warp rotation"))
	bool bIgnoreYaw = false;

	UPROPERTY(EditAnywhere, Category = "Data|Warp", meta = (ToolTip = "In regards to the desired warp translation"))
	bool bIgnoreTranslation = false;

public:
    UEnemyTargetNS();
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
};
