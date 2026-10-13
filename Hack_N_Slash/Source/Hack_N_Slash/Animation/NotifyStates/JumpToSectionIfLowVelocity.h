#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "JumpToSectionIfLowVelocity.generated.h"

UCLASS()
class HACK_N_SLASH_API UJumpToSectionIfLowVelocity : public UAnimNotifyState
{
	GENERATED_BODY()

protected:
    UPROPERTY(EditAnywhere, Category = "Jump To Section")
    FName section = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Jump To Section")
	float velocity = 100.0f;

public:
	UJumpToSectionIfLowVelocity();
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
};