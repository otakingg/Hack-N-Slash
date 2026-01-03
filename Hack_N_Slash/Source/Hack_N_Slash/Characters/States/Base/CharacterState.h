#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
//#include "GameFramework/CharacterMovementComponent.h"
#include "CharacterState.generated.h"

class ACharacter;
class UCharacterMovementComponent;
class UStateMachineComponent;
class UAnimMontage;

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

private:
    bool bInitialized {false};

protected:
    UPROPERTY()
    ACharacter* ownerChar {nullptr};

    UPROPERTY()
    UStateMachineComponent* ownerStateMachineComp {nullptr};

public:
    void Initialize(UStateMachineComponent* InSM, ACharacter* InOwner);

    /* ---------------- Lifecycle ---------------- */
    virtual void EnterState() {}
    virtual void ExitState() {}

    /* ---------------- Transition Rules ---------------- */
    virtual bool CanEnterState(const UCharacterState* PreviousState) const { return true; }
    virtual bool CanExitState() const { return true; }

    /** Priority-based interruption (still useful for Action layer) */
    virtual bool CanBeInterruptedBy(const UCharacterState* Other) const;

    /* ---------------- Metadata ---------------- */
    virtual EStatePriority GetPriority() const { return EStatePriority::Medium; }
    virtual FGameplayTag GetStateTag() const { return FGameplayTag(); }

    /* ---------------- Event Hooks (NO TICKING) ---------------- */
    virtual bool OnInputAttackPressed(const FVector2d& InputVector) { return false; }
    virtual bool OnInputBlockDodgePressed(const FVector2d& InputVector) { return false; }
    virtual bool OnInputJumpPressed() { return false; }
    virtual bool OnInputJumpReleased() { return false; }
    virtual bool OnInputLook(const FVector2d& InputVector) { return false; }
    virtual bool OnInputMove(const FVector2d& InputVector) { return false; }
    // Animation (Action + some Movement like TurnInPlace may care)
    virtual void OnAnimNotify(FName NotifyName) {}
    virtual void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted) {}
};

USTRUCT(BlueprintType)
struct FMovementInputContext
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FVector2D move {FVector2D::ZeroVector};

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FVector2D look {FVector2D::ZeroVector};

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bWantsJump {false};

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float jumpPressedTime {-1.f};

    void ClearJump()
    {
        bWantsJump = false;
        jumpPressedTime = -1.f;
    }
};

/**
 * Movement layer base.
 * Put only locomotion context here: idle/move/jump/fall/turn, etc.
 */
UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UMovementState : public UCharacterState
{
    GENERATED_BODY()

private:
    float LastGroundedTime = -1000.f;

    FTimerHandle JumpBufferTimerHandle;

    void StartJumpBufferWindow();
    void ExpireJumpBuffer();

protected:
    UPROPERTY()
    UCharacterMovementComponent* MoveComp = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FMovementInputContext InputCtx;

    /** --- Jump buffer + coyote (timer-based, no tick) --- */
    UPROPERTY(EditDefaultsOnly, Category="Movement|Tuning")
    float JumpBufferSeconds = 0.15f;

    UPROPERTY(EditDefaultsOnly, Category="Movement|Tuning")
    float CoyoteSeconds = 0.10f;

    /** --- Substate support --- */
    UPROPERTY()
    UMovementState* ActiveSubState = nullptr;

    UPROPERTY(EditDefaultsOnly, Category="Movement|Substates")
    TSubclassOf<UMovementState> DefaultGroundedStateClass;

    UPROPERTY(EditDefaultsOnly, Category="Movement|Substates")
    TSubclassOf<UMovementState> DefaultAirborneStateClass;

    //* --- Helpers --- */
    bool IsFalling() const;

    bool CanUseBufferedJump() const;
    void TryConsumeBufferedJump();

    void SetSubState(TSubclassOf<UMovementState> NewSubStateClass);
    void EvaluateBaselineSubState();

    /** --- Event hooks from movement component --- */
    UFUNCTION()
    virtual void OnLanded(const FHitResult& Hit);

    UFUNCTION()
    virtual void OnMovementModeChanged(ACharacter* InCharacter, EMovementMode PrevMovementMode, uint8 PrevCustomMode);

public:
    /* ---------------- Metadata ---------------- */
    // Movement states usually should NOT be "Critical" interrupters; override if needed.
    virtual EStatePriority GetPriority() const override { return EStatePriority::Low; }

    /* ---------------- Lifecycle ---------------- */
    virtual void EnterState() override;
    virtual void ExitState() override;

    /* ---------------- Event Hooks (NO TICKING) ---------------- */
    // Input
    //Movement typically returns false because “consuming” only matters to prevent movement from acting when action wants exclusive control
    //Can return true in special movement substates (e.g., a “TurnInPlace state consumes look”), but that’s optional
    virtual bool OnInputJumpPressed() override;
    virtual bool OnInputJumpReleased() override;
    virtual bool OnInputLook(const FVector2D& InputVector) override;
    virtual bool OnInputMove(const FVector2D& Move) override;
};

/**
 * Action layer base.
 * Put combat/guard/dodge/reactions/disabled/death here.
 */
UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UActionState : public UCharacterState
{
    GENERATED_BODY()
public:
    // Default action priority is medium; override per-state (e.g., Death=Critical).
    virtual EStatePriority GetPriority() const override { return EStatePriority::Medium; }

    /* ---------------- Event Hooks (NO TICKING) ---------------- */
    // Input
    //In UActionState you’ll override the ones you care about per concrete action
    //EX: During an attack montage, you might want to eat Move and Jump
    virtual bool OnInputJumpPressed() override;
    virtual bool OnInputJumpReleased() override;
    virtual bool OnInputLook(const FVector2D& InputVector) override;
    virtual bool OnInputMove(const FVector2D& Move) override;
};