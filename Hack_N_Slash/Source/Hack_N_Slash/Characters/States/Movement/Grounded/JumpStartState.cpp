#include "JumpStartState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UJumpStartState::EnterState()
{
	Super::EnterState();

	bImpulseApplied = false;

	if (!ownerChar || !moveComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UJumpStartState] EnterState: ownerChar or moveComp is null."));
		return;
	}

	// Consume buffered jump so the request doesn't linger (prevents stale auto-consume later)
    //Should already be done by ground container, so technically not necessary
    ConsumeBufferedJumpIfValid();

	// Apply impulse immediately unless animation-authoritative
	if (!bApplyImpulseOnNotify) ApplyJumpImpulseOnce();
}

void UJumpStartState::ExitState() { Super::ExitState(); }

//Overriden to allow JumpStart during coyote time
//Could also not have this and override in blueprint implementation, but I feel this is standard enough to warrant a C++ implementation
bool UJumpStartState::CanEnterGroundedMode_Implementation(const UCharacterState* PreviousState) const
{
    return moveComp && (moveComp->IsMovingOnGround() || ((ownerChar->GetWorld()->GetTimeSeconds() - lastGroundedTime) <= coyoteSeconds));
}

bool UJumpStartState::OnInputLook(const FVector2D& Look)
{
	if (!ownerChar) return false;

	// Record input context (buffering/debug/consistency)
	Super::OnInputLook(Look);

	if (!bAllowLookDuringJumpStart) return false;

	// Match your GroundLocomotion style exactly
	ownerChar->AddControllerYawInput(Look.X * turnRate * ownerChar->GetWorld()->GetDeltaSeconds());
	ownerChar->AddControllerPitchInput(Look.Y * lookUpRate * ownerChar->GetWorld()->GetDeltaSeconds());

	return false;
}

bool UJumpStartState::OnInputMove(const FVector2D& Move)
{
	if (!ownerChar) return false;

	// Record input context
	Super::OnInputMove(Move);

	if (bLockMovementDuringJumpStart)
	{
		// Full lock: consume input and do nothing
		if (lockedMoveScale <= KINDA_SMALL_NUMBER) return true;

		// Partial drift
		ApplyMoveInputScaled(Move, lockedMoveScale);
		return true;
	}

	// Not locked, behave like locomotion (scale 1.0)
	ApplyMoveInputScaled(Move, 1.0f);
	return false;
}

void UJumpStartState::OnAnimNotify(FName NotifyName)
{
	Super::OnAnimNotify(NotifyName);

	if (bApplyImpulseOnNotify && !bImpulseApplied && NotifyName == takeoffNotifyName) ApplyJumpImpulseOnce();
}

void UJumpStartState::ApplyMoveInputScaled(const FVector2D& Move, float Scale)
{
	if (!ownerChar || Scale <= 0.f) return;

	// Match your GroundLocomotion logic closely (one rot, zero pitch/roll)
	FRotator ControlRot = ownerChar->GetControlRotation();
	ControlRot.Pitch = 0.f;
	ControlRot.Roll  = 0.f;

	ownerChar->AddMovementInput(UKismetMathLibrary::GetRightVector(ControlRot),   Move.X * Scale);
	ownerChar->AddMovementInput(UKismetMathLibrary::GetForwardVector(ControlRot), Move.Y * Scale);
}

void UJumpStartState::ApplyJumpImpulseOnce()
{
	if (bImpulseApplied || !ownerChar || !moveComp) return;
	bImpulseApplied = true;
	moveComp->bNotifyApex = true;

	if (bUseCharacterJumpFunction)
	{
		ownerChar->Jump();
		return;
	}

	const float JumpZ = (overrideJumpZVelocity > 0.f) ? overrideJumpZVelocity : moveComp->JumpZVelocity;
	ownerChar->LaunchCharacter(FVector(0.f, 0.f, JumpZ), /*bXYOverride*/ false, /*bZOverride*/ true);
}