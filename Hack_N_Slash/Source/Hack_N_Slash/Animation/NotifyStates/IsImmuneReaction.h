#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "IsImmuneReaction.generated.h"

/**
 * 
 */
UCLASS()
class HACK_N_SLASH_API UIsImmuneReaction : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UIsImmuneReaction();
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};