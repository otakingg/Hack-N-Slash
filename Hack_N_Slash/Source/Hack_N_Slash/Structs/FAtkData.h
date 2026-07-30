#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "FAtkData.generated.h"

USTRUCT(BlueprintType)
struct FAtkData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) AActor* attacker = nullptr;
	UPROPERTY(BlueprintReadOnly) FGameplayTag attackMotionTag;
	UPROPERTY(BlueprintReadOnly) FGameplayTag attackTypeTag;
    UPROPERTY(BlueprintReadOnly) TArray<FGameplayTag> elementTags;
};