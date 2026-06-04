#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "../../Enums/EPlayerAction.h"
#include "CharacterState.generated.h"

class ACharacter;
class UCharacterMovementComponent;
class UPlayerCamComponent;
class UPlayerCombatCancelComponent;
class UPlayerCombatComponent;
class UPlayerTargettingComponent;
class UStateMachineComponent;
class UAnimMontage;
class ICombatCmdInterface;
class ILocomotionCmdInterface;
struct FAtkHitData;
struct FEnemyAtkData;

UENUM()
enum class EStatePriority : uint8
{
    Low,
    Medium,
    High,
    Critical
};

/**
 * Base State
 * - No ticking.
 * - Receives "intents" / "requests" from the StateMachineComponent
 * - Talks outward via interfaces (Locomotion/Combat) through the StateMachineComponent
 */
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
    virtual bool CanBeInterruptedBy(const UCharacterState* Other) const;

    /* ---------------- Lifecycle ---------------- */
    virtual void Initialize(UStateMachineComponent* InSM, ACharacter* InOwner);
    virtual void EnterState();
    virtual void ExitState();

    /* ---------------- Metadata ---------------- */
    virtual EStatePriority GetPriority() const { return EStatePriority::Medium; }
    virtual FGameplayTag GetStateTag() const { return stateTag; }

    /** Allow states (and containers) to contribute multiple tags */
    virtual void GatherStateTags(FGameplayTagContainer& OutTags) const
    {
        const FGameplayTag T = GetStateTag();
        if (T.IsValid()) OutTags.AddTag(T);
    }

    // "Is this state inside that tag subtree?"
    UFUNCTION(BlueprintCallable, Category="Tags")
    bool HasStateTag(const FGameplayTag& Tag) const { return stateTag.IsValid() && stateTag.MatchesTag(Tag); }

    UFUNCTION(BlueprintCallable, Category="Tags")
    bool HasExactStateTag(const FGameplayTag& Tag) const { return stateTag.IsValid() && stateTag.MatchesTagExact(Tag); }

    /* ---------------- Intent Hooks (NO TICKING) ----------------
       Return true if consumed (state machine should stop forwarding to other layer).
       Action gets first chance; Movement gets second chance.
    */

    // Action intents
    virtual bool OnAttackIntent(const FVector2D& InputVector, EPlayerAction PlayerAction) { return false; }
    virtual bool OnAttackIntent(const FEnemyAtkData& AtkData) { return false; }
    virtual bool OnBlockStartIntent() { return false; }
    virtual bool OnBlockStopIntent() { return false; }
    virtual bool OnDodgeIntent(const FVector2D& InputVector = FVector2D::ZeroVector) { return false; }
    virtual bool OnLookMouseIntent(const FVector2D& InputVector) { return false; }
    virtual bool OnLookStickIntent(const FVector2D& InputVector) { return false; }
    virtual bool OnToggleLockOnIntent() { return false; }

    // Locomotion
    virtual bool OnJumpStartIntent() { return false; }
    virtual bool OnJumpStopIntent() { return false; }
    virtual bool OnMoveIntent(const FVector2D& InputVector) { return false;}
    virtual bool OnMoveIntent(AActor* Target, const FVector& Loc = FVector::ZeroVector, float AcceptanceRadius = 50.0f) { return false; }

    // Movement feedback
    virtual void OnJumpApexReached() {}
    virtual void OnLanded(const FHitResult& Hit) {}
    virtual void OnMovementModeChanged(ACharacter* InCharacter, EMovementMode PrevMovementMode, uint8 PrevCustomMode) {}

    // Animation feedback
    virtual void OnAnimNotify(FGameplayTag NotifyTag) {}
};

UCLASS(Abstract)
class HACK_N_SLASH_API UMovementState : public UCharacterState
{
    GENERATED_BODY()

public:
    virtual EStatePriority GetPriority() const override { return EStatePriority::Low; }

    // Locomotion
    virtual bool OnMoveIntent(const FVector2D& InputVector) override;
    virtual bool OnMoveIntent(AActor* Target, const FVector& Loc = FVector::ZeroVector, float AcceptanceRadius = 50.0f) override;
};

/**
 * Action layer base.
 * Put combat/guard/dodge/reactions/disabled/death here.
 */
UCLASS(Abstract)
class HACK_N_SLASH_API UActionState : public UCharacterState
{
    GENERATED_BODY()

protected:
    UPROPERTY(Transient) UPlayerCamComponent* playerCamComp = nullptr;
    UPROPERTY(Transient) UPlayerCombatCancelComponent* playerCombatCancelComp = nullptr;
    UPROPERTY(Transient) UPlayerCombatComponent* playerCombatComp = nullptr;
    UPROPERTY(Transient) UPlayerTargettingComponent* playerTargettingComp = nullptr;
    
public:
    virtual void Initialize(UStateMachineComponent* InSM, ACharacter* InOwner) override;
    
    virtual EStatePriority GetPriority() const override { return EStatePriority::Medium; }

    // Action intents
    virtual bool OnLookMouseIntent(const FVector2D& InputVector) override;
    virtual bool OnLookStickIntent(const FVector2D& InputVector) override;
    virtual bool OnToggleLockOnIntent() override;

    // Animation feedback
    virtual void OnAnimNotify(FGameplayTag NotifyTag) override;
    
    // Combat Feedback
    virtual void ReceiveHit(const FAtkHitData& HitData) {}
};