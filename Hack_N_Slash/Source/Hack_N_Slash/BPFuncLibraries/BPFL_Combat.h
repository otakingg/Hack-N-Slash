#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "BPFL_Combat.generated.h"

/**
 * 
 */
UCLASS()
class HACK_N_SLASH_API UBPFL_Combat : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Combat|Tags")
	static bool ActorHasTag(const AActor* Actor, const FGameplayTag Tag, bool bExact = false);

	UFUNCTION(BlueprintPure, Category = "Combat|Tags")
	static bool ActorHasAnyTags(const AActor* Actor, const TArray<FGameplayTag> Tags, bool bExact = false);

	UFUNCTION(BlueprintPure, Category = "Combat|Tags")
	static bool ActorHasAllTags(const AActor* Actor, const TArray<FGameplayTag> Tags, bool bExact = false);

	UFUNCTION(BlueprintPure, Category = "Combat")
	static int32 GetActorTeamID(const AActor* Actor);

	UFUNCTION(BlueprintPure, Category = "Combat")
	static bool IsActorAlive(const AActor* Actor);
};