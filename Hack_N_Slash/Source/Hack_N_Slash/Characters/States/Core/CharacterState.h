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
 * Your unified command context (comes from the driver: Player input, AI, script, etc.)
 * This is expected to live in StateMachineComponent.h, but we forward-declare it here.
 */
struct FCommandContext;

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

    UPROPERTY()
    ACharacter* ownerChar {nullptr};

    UPROPERTY()
    UStateMachineComponent* ownerStateMachineComp {nullptr};

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
    virtual FGameplayTag GetStateTag() const { return FGameplayTag(); }

    /* ---------------- Intent Hooks (NO TICKING) ----------------
       Return true if consumed (state machine should stop forwarding to other layer).
       Action gets first chance; Movement gets second chance.
    */

    // Combat intents
    virtual bool OnAttackPressed(const FVector2D& InputVector, const FCommandContext& Ctx) { return false; }
    virtual bool OnBlockDodgePressed(const FVector2D& InputVector, const FCommandContext& Ctx) { return false; }

    // Locomotion intents
    virtual bool OnJumpPressed(const FCommandContext& Ctx);
    virtual bool OnJumpReleased(const FCommandContext& Ctx) { return false; }
    virtual bool OnLookIntent(const FVector2D& InputVector, const FCommandContext& Ctx) { return false; }
    virtual bool OnMoveIntent(const FVector2D& InputVector, const FCommandContext& Ctx) { return false; }

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

    /** --- Tuning --- */
    UPROPERTY(EditDefaultsOnly, Category="Movement|Jump")
    float jumpBufferSeconds {0.15f};

    UPROPERTY(EditDefaultsOnly, Category="Movement|Jump")
    float coyoteSeconds {0.10f};

    /** --- Jump buffer + coyote (shared) --- */
    float lastGroundedTime {1000.f};
    FTimerHandle TH_JumpBuffer;

    ILocomotionCmdInterface* GetLocoCmd() const;

    void StartJumpBufferWindow();
    UFUNCTION() void ExpireJumpBuffer();

    bool CanUseBufferedJump() const;
    void MarkGroundedNow();

public:
    virtual EStatePriority GetPriority() const override { return EStatePriority::Low; }

    virtual void Initialize(UStateMachineComponent* InSM, ACharacter* InOwner) override;

    virtual void EnterState() override;
    virtual void ExitState() override;

    // Intents (default just records; not consumed)
    virtual bool OnJumpPressed(const FCommandContext& Ctx) override;
    virtual bool OnJumpReleased(const FCommandContext& Ctx) override;
    virtual bool OnLookIntent(const FVector2D& Look, const FCommandContext& Ctx) override;
    virtual bool OnMoveIntent(const FVector2D& Move, const FCommandContext& Ctx) override;

    /**
     * Consumes buffered jump if valid right now.
     * Returns true if it was consumed (caller should trigger jump transition / execute jump).
     */
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

    // Most action states will override these as needed.
    // Return true to "eat" the intent so Movement doesn't handle it.
    virtual bool OnJumpPressed(const FCommandContext& Ctx) override { return false; }
    virtual bool OnJumpReleased(const FCommandContext& Ctx) override { return false; }
    virtual bool OnLookIntent(const FVector2D& Look, const FCommandContext& Ctx) override { return false; }
    virtual bool OnMoveIntent(const FVector2D& Move, const FCommandContext& Ctx) override { return false; }
};