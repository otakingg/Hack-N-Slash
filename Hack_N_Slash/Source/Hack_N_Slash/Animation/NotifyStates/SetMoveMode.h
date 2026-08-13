#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "SetMoveMode.generated.h"

/**
 * 
 */
UCLASS()
class HACK_N_SLASH_API USetMoveMode : public UAnimNotifyState
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Move Mode")
	TEnumAsByte<EMovementMode> startMode;

	UPROPERTY(EditAnywhere, Category = "Move Mode")
	TEnumAsByte<EMovementMode> endMode;

public:
	USetMoveMode();
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
