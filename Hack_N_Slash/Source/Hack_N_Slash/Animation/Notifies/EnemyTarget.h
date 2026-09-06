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
    UEnemyTarget();
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};