#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GameFramework/RootMotionSource.h"
#include "RMS_MoveToDyn.generated.h"

/**
 * 
 */
UCLASS()
class HACK_N_SLASH_API URMS_MoveToDyn : public UAnimNotifyState
{
	GENERATED_BODY()

protected:
    /* ---------------- GENERAL ---------------- */

    UPROPERTY(EditAnywhere, Category = "Root Motion")
    bool bDebug = false;

    UPROPERTY(EditAnywhere, Category = "Root Motion", meta = (ClampMin = 0.0f, Tooltip = "0 means duration is calculated based on distance to target, else use it"))
    float duration = 0.0f;

    UPROPERTY(EditAnywhere, Category = "Root Motion")
    float offset = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Root Motion")
	bool bRestrictSpeedToExpected = true;

    UPROPERTY(EditAnywhere, Category = "Root Motion", meta = (ClampMin = "0.1", ToolTip = "Used to calc duration, if duration isn't specified"))
    float speed = 2500.0f;

public:
	URMS_MoveToDyn();
    virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
};