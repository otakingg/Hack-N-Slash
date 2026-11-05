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
	UFUNCTION(BlueprintCallable, Category = "Combat")
	static void AddTagToActor(AActor* Actor, const FGameplayTag Tag);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	static void AddTagsToActor(AActor* Actor, const TArray<FGameplayTag> Tags);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	static void RemoveTagFromActor(AActor* Actor, const FGameplayTag Tag);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	static void RemoveTagsFromActor(AActor* Actor, const TArray<FGameplayTag> Tags);

	UFUNCTION(BlueprintPure, Category = "Combat")
	static bool ActorHasTag(const AActor* Actor, const FGameplayTag Tag, bool bExact = false);

	UFUNCTION(BlueprintPure, Category = "Combat")
	static bool ActorHasAnyTags(const AActor* Actor, const TArray<FGameplayTag> Tags, bool bExact = false);

	UFUNCTION(BlueprintPure, Category = "Combat")
	static bool ActorHasAllTags(const AActor* Actor, const TArray<FGameplayTag> Tags, bool bExact = false);

	UFUNCTION(BlueprintPure, Category = "Combat")
	static int32 GetActorTeamID(const AActor* Actor);

	UFUNCTION(BlueprintPure, Category = "Combat")
	static bool IsActorAlive(const AActor* Actor);
};