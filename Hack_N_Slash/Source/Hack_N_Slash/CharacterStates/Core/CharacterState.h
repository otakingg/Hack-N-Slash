#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "../../Tags/CharacterActionTags.h"
#include "../../Enums/EPlayerInput.h"
#include "CharacterState.generated.h"

class ACharacter;
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
    UPROPERTY(EditAnywhere, Category = "State")
    bool bDebug = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Tags")
    FGameplayTag stateTag;

    UPROPERTY(Transient, BlueprintReadOnly) ACharacter* ownerChar = nullptr;
    UPROPERTY(Transient, BlueprintReadOnly) ULocomotionComponent* locoComp = nullptr;
    UPROPERTY(Transient, BlueprintReadOnly) UCharacterMovementComponent* moveComp = nullptr;
    UPROPERTY(Transient, BlueprintReadOnly) UStateMachineComponent* ownerStateMachineComp = nullptr;

public:
    /* ---------------- Transition Rules ---------------- */
    UFUNCTION(BlueprintNativeEvent, Category = "State")
    bool CanEnterState(const UCharacterState* CurrentState) const;
    virtual bool CanEnterState_Implementation(const UCharacterState* CurrentState) const { return true; }
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

    /* ---------------- Metadata ---------------- */
    FGameplayTag GetStateTag() const { return stateTag; }

    UFUNCTION(BlueprintCallable, Category = "Tags")
    bool HasStateTag(const FGameplayTag& Tag) const { return stateTag.IsValid() && stateTag.MatchesTag(Tag); }

    UFUNCTION(BlueprintCallable, Category = "Tags")
    bool HasExactStateTag(const FGameplayTag& Tag) const { return stateTag.IsValid() && stateTag.MatchesTagExact(Tag); }

    // Movement feedback
    virtual void OnJumpApexReached() {}
    virtual void OnLanded(const FHitResult& Hit) {}
    virtual void OnMovementModeChanged(ACharacter* InCharacter, EMovementMode PrevMovementMode, uint8 PrevCustomMode) {}
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
    FGameplayTag ResolvePlayerInput(EPlayerInput PlayerInput, const FVector2D& InputVector = FVector2D::ZeroVector);
    virtual FGameplayTag ResolvePlayerInput_Implementation(EPlayerInput PlayerInput, const FVector2D& InputVector = FVector2D::ZeroVector);
    //virtual FGameplayTag ResolvePlayerInput_Implementation(EPlayerInput PlayerInput, const FVector2D& InputVector = FVector2D::ZeroVector) { return CharacterActionTags::None; }
};

/**
 * Action layer base
 */
UCLASS(Abstract)
class HACK_N_SLASH_API UActionState : public UCharacterState
{
    GENERATED_BODY()

public:
    virtual void OnAnimNotify(FGameplayTag NotifyTag); // Animation Feedback
    virtual void ReceiveHit(const FAtkHitData& HitData) {} // Combat Feedback

    // Player only: Action Management
    // The action layer decides what the action means
    // EX: NoneState | Jump Action Started = Jump Start
    // EX: HitState | Jump Action Started = Play recover animation if in knockback animation and pressed just before hitting the ground
    UFUNCTION(BlueprintNativeEvent, Category = "State")
    FGameplayTag ResolvePlayerAction(const FGameplayTag& PlayerAction, const FVector2D& InputVector = FVector2D::ZeroVector);
    virtual FGameplayTag ResolvePlayerAction_Implementation(const FGameplayTag& PlayerAction, const FVector2D& InputVector = FVector2D::ZeroVector) { return PlayerAction; }
};