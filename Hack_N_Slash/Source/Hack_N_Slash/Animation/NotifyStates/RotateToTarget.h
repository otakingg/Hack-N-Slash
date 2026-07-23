#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "RotateToTarget.generated.h"

/**
 * 
 */
UCLASS()
class HACK_N_SLASH_API URotateToTarget : public UAnimNotifyState
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditAnywhere, Category = "Rotate")
	float roationSpeed = 20.0f;

	UPROPERTY(EditAnywhere, Category = "Rotate")
	bool bRotPitch = false;

	UPROPERTY(EditAnywhere, Category = "Rotate")
	bool bRotRoll = false;

public:
	URotateToTarget();
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
};
