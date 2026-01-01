#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CharacterState.generated.h"

class ACharacter;
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
    // Movement / physics
    virtual void OnLanded(const FHitResult& Hit) {}
    virtual void OnMovementModeChanged(EMovementMode PrevMode, uint8 PrevCustomMode) {}

    // Input (usually only Action states care)
    virtual void OnInputAttackPressed() {}
    virtual void OnInputBlockDodgePressed() {}
    virtual void OnInputJumpPressed() {}
    virtual void OnInputJumpReleased() {}
    virtual void OnInputMoveStarted() {}
    virtual void OnInputMoveStopped() {}

    // Animation (Action + some Movement like TurnInPlace may care)
    virtual void OnAnimNotify(FName NotifyName) {}
    virtual void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted) {}
};

/**
 * Movement layer base.
 * Put only locomotion context here: idle/move/jump/fall/turn, etc.
 */
UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UMovementState : public UCharacterState
{
    GENERATED_BODY()
public:
    // Movement states usually should NOT be "Critical" interrupters; override if needed.
    virtual EStatePriority GetPriority() const override { return EStatePriority::Low; }
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
};