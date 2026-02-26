#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "CharacterState.generated.h"

class ACharacter;
class UCharacterMovementComponent;
class UStateMachineComponent;
class UAnimMontage;
class ILocomotionCmdInterface;

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
    bool bInitialized {false};

protected:
    UPROPERTY(EditAnywhere)
    bool bDebug {false};

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Tags")
    FGameplayTag stateTag;

    UPROPERTY() ACharacter* ownerChar {nullptr};

    UPROPERTY() UStateMachineComponent* ownerStateMachineComp {nullptr};

    /** Camera tuning (optional) */
    UPROPERTY(EditDefaultsOnly, Category="Camera", meta=(ClampMin="0.0"))
    float lookUpRate {45.f};

    UPROPERTY(EditDefaultsOnly, Category="Camera", meta=(ClampMin="0.0"))
    float turnRate {45.f};

public:
    virtual void Initialize(UStateMachineComponent* InSM, ACharacter* InOwner);

    /* ---------------- Lifecycle ---------------- */
    virtual void EnterState();
    virtual void ExitState() {}

    /* ---------------- Transition Rules ---------------- */
    virtual bool CanEnterState(const UCharacterState* PreviousState) const { return true; }
    virtual bool CanExitState() const { return true; }

    /** Priority-based interruption (still useful for Action layer) */
    virtual bool CanBeInterruptedBy(const UCharacterState* Other) const;

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

    // Combat intents
    virtual bool OnAttackIntent(const FVector2D& InputVector) { return false; }
    virtual bool OnBlockStartIntent() { return false; }
    virtual bool OnBlockStopIntent() { return false; }
    virtual bool OnDodgeIntent(const FVector2D& InputVector) { return false; }

    // Locomotion intents
    virtual bool OnJumpPressed();
    virtual bool OnJumpReleased() { return false; }
    virtual bool OnLookIntent(const FVector2D& InputVector) { return false; }
    virtual bool OnMoveIntent(const FVector2D& InputVector) { return false; }
    virtual bool OnMoveIntent(AActor* Target, const FVector& Loc, const FGameplayTag& MoveProfile, float AcceptanceRadius = 50.0f) { return false; }

    // Animation feedback (Action + some Movement like TurnInPlace may care)
    virtual void OnAnimNotify(FName NotifyName) {}
    virtual void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted) {}
};

USTRUCT(BlueprintType)
struct FMovementInputContext
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FVector2D Move {FVector2D::ZeroVector};

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FVector2D Look {FVector2D::ZeroVector};

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bWantsJump {false};

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float JumpPressedTime {-1.f};

    void ClearJump()
    {
        bWantsJump = false;
        JumpPressedTime = -1.f;
    }
};

UCLASS(Abstract)
class HACK_N_SLASH_API UMovementState : public UCharacterState
{
    GENERATED_BODY()

protected:
    UPROPERTY()
    UCharacterMovementComponent* moveComp {nullptr};

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Movement")
    FMovementInputContext inputCtx;

    FTimerHandle TH_ClearAirborne;

    void ClearAirborneModeDelayed();
    
    ILocomotionCmdInterface* GetLocoCmd() const;

public:
    virtual EStatePriority GetPriority() const override { return EStatePriority::Low; }

    virtual void Initialize(UStateMachineComponent* InSM, ACharacter* InOwner) override;

    virtual void EnterState() override;
    virtual void ExitState() override {}

    // Intents (default just records; not consumed)
    virtual bool OnJumpPressed() override;
    virtual bool OnJumpReleased() override;
    virtual bool OnLookIntent(const FVector2D& InputVector) override;
    virtual bool OnMoveIntent(const FVector2D& InputVector) override;
    virtual bool OnMoveIntent(AActor* Target, const FVector& Loc, const FGameplayTag& MoveProfile, float AcceptanceRadius = 50.0f) override;

    /** Consumes buffered jump if valid right now. */
    bool ConsumeBufferedJumpIfValid();

    // Forwarded by component
    virtual void OnJumpApexReached() {}
    virtual void OnLanded(const FHitResult& Hit) {}
    virtual void OnMovementModeChanged(ACharacter* InCharacter, EMovementMode PrevMovementMode, uint8 PrevCustomMode) {}
};

/**
 * Action layer base.
 * Put combat/guard/dodge/reactions/disabled/death here.
 */
UCLASS(Abstract)
class HACK_N_SLASH_API UActionState : public UCharacterState
{
    GENERATED_BODY()

public:
    virtual EStatePriority GetPriority() const override { return EStatePriority::Medium; }
};