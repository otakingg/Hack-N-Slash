#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "../../Utility/Tags.h"
#include "../../Enums/EPlayerInput.h"
#include "CharacterState.generated.h"

class ACharacter;
class ICombatInstigator;
class UBaseCharAnimInstance;
class UCharacterMovementComponent;
class ULocomotionComponent;
class UStateMachineComponent;
struct FAtkHitData;

UCLASS(Abstract)
class HACK_N_SLASH_API UCharacterState : public UObject
{
    GENERATED_BODY()

private:
    bool bInitialized = false;
    
protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    bool bDebug = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "State")
    FGameplayTag stateTag;

    UPROPERTY(Transient, BlueprintReadOnly) UBaseCharAnimInstance* animInst = nullptr;
    UPROPERTY(Transient, BlueprintReadOnly) ACharacter* ownerChar = nullptr;
    UPROPERTY(Transient, BlueprintReadOnly) ULocomotionComponent* locoComp = nullptr;
    UPROPERTY(Transient, BlueprintReadOnly) UCharacterMovementComponent* moveComp = nullptr;
    UPROPERTY(Transient, BlueprintReadOnly) UStateMachineComponent* ownerStateMachineComp = nullptr;
    ICombatInstigator* iCmbtInst = nullptr;

    UFUNCTION(BlueprintPure, Category = "State")
    bool HasGameplayTag(const FGameplayTag& Tag, bool bExact = false) const;

    UFUNCTION(BlueprintPure, Category = "State")
    bool HasAnyGameplayTag(const TArray<FGameplayTag>& Tags, bool bExact = false) const;

public:
    /* ---------------- Transition Rules ---------------- */
    UFUNCTION(BlueprintNativeEvent, Category = "State")
    bool CanEnterState(const UCharacterState* CurrentState) const;
    virtual bool CanEnterState_Implementation(const UCharacterState* CurrentState) const { return ownerStateMachineComp && ownerChar && moveComp; }
    virtual bool CanExitState() const { return true; }

    /* ---------------- Lifecycle ---------------- */
    UFUNCTION(BlueprintNativeEvent, Category = "State")
    void Initialize(UStateMachineComponent* InSM, ACharacter* Owner);
    virtual void Initialize_Implementation(UStateMachineComponent* InSM, ACharacter* InOwner);

    UFUNCTION(BlueprintNativeEvent, Category = "State")
    void EnterState();
    virtual void EnterState_Implementation();

    UFUNCTION(BlueprintNativeEvent, Category = "State")
    void ExitState();
    virtual void ExitState_Implementation();

    /* ---------------- Tags ---------------- */
    FGameplayTag GetStateTag() const { return stateTag; }
};

UCLASS(Abstract)
class HACK_N_SLASH_API UMovementState : public UCharacterState
{
    GENERATED_BODY()

public:
    // Player only: Input Management
    // The movement layer decides what player input actually means
    // EX: Grounded | West Face Button Started = Light Attack
    // EX: Climbing | West Face Button Stared = Stop Climbing
    UFUNCTION(BlueprintNativeEvent, Category = "State")
    FGameplayTag ResolvePlayerInput(EPlayerInput PlayerInput, const FVector2D& LookVector = FVector2D::ZeroVector, const FVector2D& MoveVector = FVector2D::ZeroVector);
    virtual FGameplayTag ResolvePlayerInput_Implementation(EPlayerInput PlayerInput, const FVector2D& LookVector = FVector2D::ZeroVector, const FVector2D& MoveVector = FVector2D::ZeroVector);
};

/**
 * Action layer base
 */
UCLASS(Abstract)
class HACK_N_SLASH_API UActionState : public UCharacterState
{
    GENERATED_BODY()

public:
    // Movement feedback
    UFUNCTION(BlueprintNativeEvent, Category = "State")
    void OnJumpApexReached();
    virtual void OnJumpApexReached_Implementation() {}
    virtual void OnLanded(const FHitResult& Hit) {}
    virtual void OnMovementModeChanged(ACharacter* InCharacter, EMovementMode PrevMovementMode, uint8 PrevCustomMode) {}
    
    // Animation Feedback
    UFUNCTION(BlueprintNativeEvent, Category = "State")
    void OnAnimNotify(FGameplayTag NotifyTag);
    virtual void OnAnimNotify_Implementation(FGameplayTag NotifyTag);

    // Combat Feedback
    UFUNCTION(BlueprintNativeEvent, Category = "State")
    void ReceiveHit(const FAtkHitData& HitData);
    virtual void ReceiveHit_Implementation(const FAtkHitData& HitData) {}

    // Player only: Action Management
    // The action layer decides what the action means
    // EX: NoneState | Jump Action Started = Jump Start
    // EX: HitState | Jump Action Started = Play recover animation if in knockback animation and pressed just before hitting the ground
    UFUNCTION(BlueprintNativeEvent, Category = "State")
    FGameplayTag ResolvePlayerAction(const FGameplayTag& PlayerAction);
    virtual FGameplayTag ResolvePlayerAction_Implementation(const FGameplayTag& PlayerAction) { return PlayerAction; }
};