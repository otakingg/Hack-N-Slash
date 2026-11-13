#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "../../Structs/FAtkHitData.h"
#include "SetReactionOverride.generated.h"

/**
 * 
 */
UCLASS()
class HACK_N_SLASH_API USetReactionOverride : public UAnimNotifyState
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Reaction")
	EHitReaction reactionOverride = EHitReaction::None;

public:
	USetReactionOverride();
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
