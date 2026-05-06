#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "RMS_MTD.generated.h"

class ILocomotionCmdInterface;

/**
 * 
 */
UCLASS()
class HACK_N_SLASH_API URMS_MTD : public UAnimNotifyState
{
	GENERATED_BODY()

private:
	ILocomotionCmdInterface* iLocoCmd = nullptr;
	AActor* owner = nullptr;
	AActor* target = nullptr;

protected:
    UPROPERTY(EditAnywhere, Category = "Root Motion")
    bool bDebug = false;

	UPROPERTY(EditAnywhere, Category = "Root Motion")
	float roationSpeed = 20.0f;

    UPROPERTY(EditAnywhere, Category = "Root Motion")
    float offset = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Root Motion")
	bool bRestrictSpeedToExpected = true;

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
};
