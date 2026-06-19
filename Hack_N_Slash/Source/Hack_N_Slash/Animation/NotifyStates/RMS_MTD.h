#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "RMS_MTD.generated.h"

/**
 * 
 */
UCLASS()
class HACK_N_SLASH_API URMS_MTD : public UAnimNotifyState
{
	GENERATED_BODY()

protected:
    UPROPERTY(EditAnywhere, Category = "Root Motion")
    bool bDebug = false;

	UPROPERTY(EditAnywhere, Category = "Root Motion")
	float roationSpeed = 20.0f;

    UPROPERTY(EditAnywhere, Category = "Root Motion", meta = (ClampMin = 0.0f, Tooltip = "0 means duration is calculated based on distance to target, else use it"))
    float duration = 0.5f;

    UPROPERTY(EditAnywhere, Category = "Root Motion")
    float offset = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Root Motion")
	bool bRestrictSpeedToExpected = true;

public:
	URMS_MTD();
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
};
