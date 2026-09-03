#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ModifyPoise.generated.h"

UENUM(BlueprintType)
enum class EPoiseModStyle : uint8
{
    None,
	Add,
    Set
};

/**
 * 
 */
UCLASS()
class HACK_N_SLASH_API UModifyPoise : public UAnimNotifyState
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Poise")
	EPoiseModStyle modStyle = EPoiseModStyle::None;

	UPROPERTY(EditAnywhere, Category = "Poise", meta = (EditCondition = "ModStyle == EPoiseModStyle::Add", EditConditionHides))
	float additionalPoise = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Poise", meta = (EditCondition = "ModStyle == EPoiseModStyle::Set", EditConditionHides))
	float newPoise = 0.0f;
	
public:
	UModifyPoise();
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};