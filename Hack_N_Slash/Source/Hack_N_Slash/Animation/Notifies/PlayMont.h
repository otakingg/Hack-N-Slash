#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "PlayMont.generated.h"

/**
 * 
 */
UCLASS()
class HACK_N_SLASH_API UPlayMont : public UAnimNotify
{
	GENERATED_BODY()

protected:
    UPROPERTY(EditAnywhere, Category = "Play Montage")
    UAnimMontage* montage = nullptr;

	UPROPERTY(EditAnywhere, Category = "Play Montage")
	FName section = NAME_None;

public:
    UPlayMont();
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};