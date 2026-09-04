#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AirFloat.generated.h"

/**
 * Modifies the owner's movement properties to float in the air
 */
UCLASS()
class HACK_N_SLASH_API UAirFloat : public UAnimNotifyState
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Air Float")
	float gravity = 0.2f;

	UPROPERTY(EditAnywhere, Category = "Air Float")
	float minFallSpeed = -100.0f;

	UPROPERTY(EditAnywhere, Category = "Air Float")
	bool bRestrictUpwardVelocity = true;
	
public:
	UAirFloat();
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};