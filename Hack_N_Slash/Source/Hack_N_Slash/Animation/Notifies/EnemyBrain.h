#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "EnemyBrain.generated.h"

/**
 * 
 */
UCLASS()
class HACK_N_SLASH_API UEnemyBrain : public UAnimNotify
{
	GENERATED_BODY()

protected:
    UPROPERTY(EditAnywhere, Category = "AI Brain")
    TArray<FGameplayTag> notifyTags;

public:
    UEnemyBrain();
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};