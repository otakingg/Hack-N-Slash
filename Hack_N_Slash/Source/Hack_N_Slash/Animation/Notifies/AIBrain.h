#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "AIBrain.generated.h"

/**
 * Sends events to the enemy brain
 */
UCLASS()
class HACK_N_SLASH_API UAIBrain : public UAnimNotify
{
	GENERATED_BODY()

protected:
    UPROPERTY(EditAnywhere, Category = "AI Brain")
    TArray<FGameplayTag> notifyTags;

public:
    UAIBrain();
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
