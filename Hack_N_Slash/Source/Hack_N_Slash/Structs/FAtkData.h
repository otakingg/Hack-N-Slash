#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "FAtkData.generated.h"

USTRUCT(BlueprintType)
struct FAtkData
{
	GENERATED_BODY()

	AActor* attacker = nullptr;
    TArray<FGameplayTag> attackTags;
    TArray<FGameplayTag> elementTags;
};