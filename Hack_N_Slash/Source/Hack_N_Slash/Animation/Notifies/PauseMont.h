#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "PauseMont.generated.h"

/**
 * Pauses the current montage
 */
UCLASS()
class HACK_N_SLASH_API UPauseMont : public UAnimNotify
{
	GENERATED_BODY()

protected:
    UPROPERTY(EditAnywhere, Category = "Pause")
    bool bDebug = false;

public:
    UPauseMont();
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
