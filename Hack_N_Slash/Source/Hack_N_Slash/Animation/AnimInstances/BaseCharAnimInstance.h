#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameplayTagContainer.h"
#include "BaseCharAnimInstance.generated.h"

// Handles the base animation logic for the characters in this game

class ACharacter;
class ICombatInstigator;
class UCharacterMovementComponent;

USTRUCT(BlueprintType)
struct FCharAnimData // Contains the data the animation system needs
{
    GENERATED_BODY()

    // --- Ownership ---
    UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Owner")
    ACharacter* character = nullptr;

    UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Owner")
    UCharacterMovementComponent* moveComp = nullptr;

    // --- Movement basics ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Movement")
    FVector velocity = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Movement")
    FVector accel = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Movement")
    float speed = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Movement")
    float speed2D = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Movement")
    bool bHasAcceleration = false;

    // --- Tags / State context ---
    UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Tags")
    FGameplayTagContainer stateTags; 
};

UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UBaseCharAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim")
    FCharAnimData animData;

    ICombatInstigator* iCmbtInst = nullptr;

    void CacheOwner();
    void BuildMovementData();
    void BuildTags();

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim")
    bool bDebug = false;

    UFUNCTION(BlueprintCallable, Category="Anim")
    void InitializeAnimation();

    UFUNCTION(BlueprintCallable, Category="Anim")
    void UpdateAnimation(float DeltaSeconds);

    UFUNCTION(BlueprintPure, Category="Anim")
    bool HasStateTag(const FGameplayTag& Tag, bool bExact = false) const;
    
    UFUNCTION(BlueprintCallable, Category = "Anim")
    float PlayMontageHNS(UAnimMontage* Montage = nullptr, FName Section = NAME_None);
};