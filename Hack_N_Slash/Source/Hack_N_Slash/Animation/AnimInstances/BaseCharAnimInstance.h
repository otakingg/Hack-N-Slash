// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "BaseCharAnimInstance.generated.h"

struct FGameplayTag;
class UCharacterMovementComponent;
class UStateMachineComponent;

/**
 * 
 */
UCLASS()
class HACK_N_SLASH_API UBaseCharAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
    UPROPERTY(BlueprintReadOnly)
    APawn* owningPawn;

	UPROPERTY(BlueprintReadOnly)
	UCharacterMovementComponent* movementComp;

    UPROPERTY(BlueprintReadOnly)
    UStateMachineComponent* stateMachineComp;

    UPROPERTY(BlueprintReadOnly)
    FVector velocity;

    UPROPERTY(BlueprintReadOnly)
    float speed;

    UPROPERTY(BlueprintReadOnly)
    bool bIsFalling;

public:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    /* ---- State Info ---- */

    UFUNCTION(BlueprintCallable)
    FGameplayTag GetActiveStateTag() const;

    //UFUNCTION(BlueprintCallable)
    //bool IsInStateTag(FGameplayTag StateTag) const;

    /* ---- Combat ---- */

    //UFUNCTION(BlueprintCallable)
    //bool IsAttacking() const;

    //UFUNCTION(BlueprintCallable)
    //bool IsInAerialCombat() const;

    //UFUNCTION(BlueprintCallable)
    //bool IsBlocking() const;

    /* ---- Hit / Death ---- */

    //UFUNCTION(BlueprintCallable)
    //bool IsHitReacting() const;

    //UFUNCTION(BlueprintCallable)
    //bool IsDead() const;

    /* ---- Animation Control ---- */

    //UFUNCTION(BlueprintCallable)
    //UAnimMontage* GetActiveMontage() const;
};
