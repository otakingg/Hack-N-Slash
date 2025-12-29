// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "CharacterState.generated.h"

class ACharacter;
class UStateMachineComponent;

UENUM()
enum class EStatePriority : uint8
{
    Low,
    Medium,
    High,
    Critical    // Death, stun lock, cinematic, etc
};

UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UCharacterState : public UObject
{
	GENERATED_BODY()

protected:
	UPROPERTY()
	ACharacter* ownerCharacter;

    UPROPERTY()
    UStateMachineComponent* ownerStateMachine;

public:
    void Initialize(UStateMachineComponent* InSM, ACharacter* Owner);

    /** Lifecycle */
    virtual void EnterState() {}
    virtual void ExitState() {}
    virtual void TickState(float DeltaTime) {}

    /** Transition rules */
    virtual bool CanEnterState() const { return false; }
    virtual bool CanExitState() const { return false; }
    virtual bool CanBeInterruptedBy(const UCharacterState*) const;

    /** Metadata */
    virtual EStatePriority GetPriority() const { return EStatePriority::Medium; }
    virtual FGameplayTag GetStateTag() const { return FGameplayTag(); }
};