#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "ResetCombo.generated.h"

/**
 * 
 */
UCLASS()
class HACK_N_SLASH_API UResetCombo : public UAnimNotify
{
	GENERATED_BODY()

public:
    UResetCombo();
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};