// BaseCharAnimInstance.h
#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameplayTagContainer.h"
#include "../../Interfaces/CharAnimInterface.h"
#include "BaseCharAnimInstance.generated.h"

class UStateMachineComponent;

USTRUCT(BlueprintType)
struct FCharAnimData
{
    GENERATED_BODY()

    // --- Ownership ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Anim|Owner")
    TObjectPtr<class ACharacter> Character = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Anim|Owner")
    TObjectPtr<class UCharacterMovementComponent> MoveComp = nullptr;

    // --- Movement basics ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Anim|Movement")
    FVector VelocityWS = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Anim|Movement")
    FVector AccelWS = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Anim|Movement")
    float Speed = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Anim|Movement")
    float Speed2D = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Anim|Movement")
    bool bHasAcceleration = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Anim|Movement")
    bool bIsFalling = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Anim|Movement")
    bool bIsGrounded = false;

    // --- Tags / State context ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Anim|Tags")
    FGameplayTagContainer StateTags;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Anim|Tags", meta = (Tooltip = "Optional extra 'anim-only' context tags"))
    FGameplayTagContainer AnimContextTags;
};

UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UBaseCharAnimInstance : public UAnimInstance, public ICharAnimInterface
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

    FVector PrevVelocityWS {FVector::ZeroVector};

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