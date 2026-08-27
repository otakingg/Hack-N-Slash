#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CombatInstigator.generated.h"

struct FGameplayTag;
struct FGameplayTagContainer;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTagsUpdated);

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UCombatInstigator : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class HACK_N_SLASH_API ICombatInstigator
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	//virtual const FGameplayTagContainer& GetTags() const = 0;
	virtual const TMap<FGameplayTag, int32>& GetTags() const = 0;
	virtual void AddTag(const FGameplayTag& Tag) {}
	virtual void RemoveTag(const FGameplayTag& Tag) {}
	virtual bool HasTag(const FGameplayTag& Tag, bool bExact = false) const { return false; }
	//virtual bool HasAnyTag(const FGameplayTagContainer& TagContainer, bool bExact = false) const { return false; }
	virtual bool HasAnyTag(const TArray<FGameplayTag>& TagArray, bool bExact = false) const { return false; }
	//virtual bool HasAllTags(const FGameplayTagContainer& TagContainer, bool bExact = false) const { return false; }
	virtual bool HasAllTags(const TArray<FGameplayTag>& TagArray, bool bExact = false) const { return false; }
	virtual bool IsAirborne() const { return false; } // Based on tags
	virtual bool IsGrounded() const { return false; } // Based on tags

	virtual AActor* GetCurrentTarget() const { return nullptr; }
	virtual bool GetLockedOn() const { return false; }
};