#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "OpenAtkDelayWindow.generated.h"

/**
 * 
 */
UCLASS()
class HACK_N_SLASH_API UOpenAtkDelayWindow : public UAnimNotify
{
	GENERATED_BODY()

public:
    UOpenAtkDelayWindow();
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};