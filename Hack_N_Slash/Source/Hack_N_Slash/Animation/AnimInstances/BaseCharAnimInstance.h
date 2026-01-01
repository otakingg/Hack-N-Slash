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

private:
    // If you want to keep a dead-simple “don’t update” guard
	bool bInitialized {false};

protected:
	// Cache pointers once
	UPROPERTY(Transient)
	TObjectPtr<ACharacter> charOwner {nullptr};

	UPROPERTY(Transient)
	TObjectPtr<UCharacterMovementComponent> moveComp {nullptr};

	UPROPERTY(Transient)
	TObjectPtr<UStateMachineComponent> stateMachineComp {nullptr};

    // ---- Common “Locomotion inputs” for Motion Matching / graphs ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Locomotion")
    float acceleration;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Locomotion")
    FVector velocity;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Locomotion")
    float speed;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Locomotion")
    bool bIsFalling {false};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Locomotion")
	bool bHasAcceleration {false};

	// Internals
	void CacheOwnerRefs();
	void UpdateLocomotionData(float DeltaSeconds);

public:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

// ---- Owner access ----
	UFUNCTION(BlueprintPure, Category="Owner")
	ACharacter* GetCharacterOwnerCached() const { return charOwner; }

	UFUNCTION(BlueprintPure, Category="Owner")
	UCharacterMovementComponent* GetMoveCompCached() const { return moveComp; }

	UFUNCTION(BlueprintPure, Category="Owner")
	UStateMachineComponent* GetStateMachineCached() const { return stateMachineComp; }

	// ---- Montage helpers (generic, not “player-only”) ----
	float PlayActionMontage(UAnimMontage* Montage, float PlayRate = 1.f, FName StartSection = NAME_None);
	void StopAllMontages(float BlendOutTime = 0.15f);
};
