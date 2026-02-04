// BaseCharAnimInstance.h
#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
//#include "GameplayTagContainer.h"
#include "../../Structs/FCharAnimData.h"
#include "BaseCharAnimInstance.generated.h"

class UStateMachineComponent;

UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UBaseCharAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

protected:
    // Cached owner
    UPROPERTY(Transient)
    TObjectPtr<ACharacter> CachedCharacter {nullptr};

    UPROPERTY(Transient)
    TObjectPtr<UCharacterMovementComponent> CachedMoveComp {nullptr};

	UPROPERTY(Transient)
	TObjectPtr<UStateMachineComponent> CachedStateMachineComp {nullptr};

    // --- What AnimBP reads ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Anim")
    FCharAnimData AnimData;

    void CacheOwner();

    void BuildMovementData(float DeltaSeconds);
    void BuildTags();

    // Hook: override in child anim instance if you need custom context tags
    virtual void GatherAnimContextTags(FGameplayTagContainer& OutTags) const;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Anim|Debug")
    bool bDebug {false};

    // --- Core ---
    //virtual void NativeInitializeAnimation() override;
    //virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	UFUNCTION(BlueprintCallable, Category="Anim")
    void InitializeAnimation();
	UFUNCTION(BlueprintCallable, Category="Anim")
    void UpdateAnimation(float DeltaSeconds);
	
    // --- Queries usable in AnimBP ---
    UFUNCTION(BlueprintPure, Category="Anim|Tags")
    bool HasStateTag(FGameplayTag Tag) const;

    UFUNCTION(BlueprintPure, Category="Anim|Tags")
    bool HasAnyStateTags(const FGameplayTagContainer& Tags) const;
};