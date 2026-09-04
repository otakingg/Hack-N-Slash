#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "ClearAtkData.generated.h"

/**
 * Clears owner's current attack data
 */
UCLASS()
class HACK_N_SLASH_API UClearAtkData : public UAnimNotify
{
	GENERATED_BODY()

public:
    UClearAtkData();
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
