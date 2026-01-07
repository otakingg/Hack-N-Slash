#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
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

UCLASS(Abstract)
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

UCLASS(Abstract)
class HACK_N_SLASH_API UMovementState : public UCharacterState
{
    GENERATED_BODY()

protected:
    UPROPERTY()
    UCharacterMovementComponent* moveComp {nullptr};

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FMovementInputContext inputCtx;

public:
    virtual EStatePriority GetPriority() const override { return EStatePriority::Low; }

    virtual void EnterState() override;
    virtual void ExitState() override;

    // Input (leaf/container/root can override; default just records in ctx and returns false)
    virtual bool OnInputJumpPressed() override;
    virtual bool OnInputJumpReleased() override;
    virtual bool OnInputLook(const FVector2D& InputVector) override;
    virtual bool OnInputMove(const FVector2D& Move) override;

    // Optional hooks containers/leaf can override (ROOT will call these on active container)
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