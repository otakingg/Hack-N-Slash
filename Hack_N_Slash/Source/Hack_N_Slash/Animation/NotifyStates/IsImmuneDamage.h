#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "IsImmuneDamage.generated.h"

/**
 * 
 */
UCLASS()
class HACK_N_SLASH_API UIsImmuneDamage : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UIsImmuneDamage();
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};