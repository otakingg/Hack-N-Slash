#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "../../Enums/EButtonInput.h"
#include "../../Enums/ECharacterAction.h"
#include "CharacterState.generated.h"

class ACharacter;
class UCharacterMovementComponent;
class UStateMachineComponent;
class ICombatCmdInterface;
class ILocomotionCmdInterface;
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

    UPROPERTY(Transient) ACharacter* ownerChar = nullptr;
    UPROPERTY(Transient) UCharacterMovementComponent* moveComp = nullptr;
    UPROPERTY(Transient) UStateMachineComponent* ownerStateMachineComp = nullptr;

    ICombatCmdInterface* GetCombatCmd() const;
    ILocomotionCmdInterface* GetLocoCmd() const;

public:
    /* ---------------- Transition Rules ---------------- */
    UFUNCTION(BlueprintNativeEvent, Category = "State")
    bool CanEnterState(const UCharacterState* PreviousState) const;
    virtual bool CanEnterState_Implementation(const UCharacterState* PreviousState) const { return true; }
    virtual bool CanExitState() const { return true; }

    /* ---------------- Lifecycle ---------------- */
    virtual void Initialize(UStateMachineComponent* InSM, ACharacter* InOwner);
    virtual void EnterState();
    virtual void ExitState();

    /* ---------------- Metadata ---------------- */
    virtual FGameplayTag GetStateTag() const { return stateTag; }

    UFUNCTION(BlueprintCallable, Category="Tags")
    bool HasStateTag(const FGameplayTag& Tag) const { return stateTag.IsValid() && stateTag.MatchesTag(Tag); }

    UFUNCTION(BlueprintCallable, Category="Tags")
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
    // Player only
    // The movement layer decides what player input actually means
    // EX: Grounded | West Face Button Started = Light Attack
    // EX: Climbing | West Face Button Stared = Stop Climbing
    UFUNCTION(BlueprintNativeEvent, Category = "State")
    ECharacterAction ResolveButtonInput(EButtonInput ButtonInput, const FVector2D& InputVector = FVector2D::ZeroVector);
    virtual ECharacterAction ResolveButtonInput_Implementation(EButtonInput ButtonInput, const FVector2D& InputVector = FVector2D::ZeroVector) { return ECharacterAction::None; }
};

/**
 * Action layer base
 */
UCLASS(Abstract)
class HACK_N_SLASH_API UActionState : public UCharacterState
{
    GENERATED_BODY()

public:
    // Animation feedback
    virtual void OnAnimNotify(FGameplayTag NotifyTag);
    
    // Combat Feedback
    virtual void ReceiveHit(const FAtkHitData& HitData) {}

    virtual bool TryCharacterAction(ECharacterAction Action, const FVector2D& InputVector) { return false; }
};