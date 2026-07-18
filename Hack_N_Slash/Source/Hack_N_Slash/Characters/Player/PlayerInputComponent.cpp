#include "PlayerInputComponent.h"
#include "../../Interfaces/CombatInstigator.h"
#include "Player_Base.h"
#include "../Shared/StateMachineComponent.h"

UPlayerInputComponent::UPlayerInputComponent() { PrimaryComponentTick.bCanEverTick = true; }

void UPlayerInputComponent::BeginPlay()
{
	Super::BeginPlay();

	player = Cast<APlayer_Base>(GetOwner());
	stateMachineComp = player ? player->FindComponentByClass<UStateMachineComponent>() : nullptr;
	iCmbtInst = Cast<ICombatInstigator>(player);
}

void UPlayerInputComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bufferedAction.time < 0.0f)
	{
		ClearActionBuffer();
		return;
	}

	UWorld* world = GetWorld();
	if (!world) return;

	if (!player) player = Cast<APlayer_Base>(GetOwner());
	if (!player) return;

	float timeSinceInputAction = world->GetTimeSeconds() - bufferedAction.time;
	if (timeSinceInputAction > actionBufferMaxTime) ClearActionBuffer();
	else player->TryBufferedAction(bufferedAction.action, bufferedAction.move);
}

FVector UPlayerInputComponent::GetInputWorldDirRelativeToCamOrTarget(const FVector2D& InputVector, FVector& OutLocalForward, FVector& OutLocalRight, AActor* Target) const
{
	if (InputVector.IsNearlyZero()) return FVector::ZeroVector;

    if (Target && iCmbtInst && iCmbtInst->GetLockedOn()) // Calculate direction relative to the target
    {
        OutLocalForward = Target->GetActorLocation() - player->GetActorLocation(); // The new "forward"
        OutLocalForward.Z = 0.f; // Flatten to horizontal plane
        OutLocalForward.Normalize(); // Normalize because we only care about direction

		// Cross product generates a perpendicular direction
		// Gives the right by crossing up with forward
        OutLocalRight = FVector::CrossProduct(FVector::UpVector, OutLocalForward).GetSafeNormal();
    }
    else // Calculate direction relative to the camera
    {
		// Only use camera yaw
        const FRotator controlRot = player->GetControlRotation();
        const FRotator yawOnlyRot(0.f, controlRot.Yaw, 0.f);

        OutLocalForward = FRotationMatrix(yawOnlyRot).GetUnitAxis(EAxis::X); // Camera forward
        OutLocalRight   = FRotationMatrix(yawOnlyRot).GetUnitAxis(EAxis::Y); // Camera right
    }

	FVector inputWorldDir = (OutLocalForward * InputVector.Y) + (OutLocalRight * InputVector.X); // Converts stick input into world direction
	inputWorldDir.Z = 0.f;
	return inputWorldDir.GetSafeNormal(); // Normalize because we only care about direction
}

EStickDirection UPlayerInputComponent::GetStickDirFromWorldDir(const FVector& WorldDir, const FVector& LocalForward, const FVector& LocalRight) const
{
	if (WorldDir.IsNearlyZero() || LocalForward.IsNearlyZero() || LocalRight.IsNearlyZero()) return EStickDirection::Neutral;

	FVector moveDir = WorldDir;
	moveDir.Z = 0.f;
	moveDir = moveDir.GetSafeNormal();

	FVector forward = LocalForward;
	forward.Z = 0.f;
	forward = forward.GetSafeNormal();

	FVector right = LocalRight;
	right.Z = 0.f;
	right = right.GetSafeNormal();

	const float forwardDot = FVector::DotProduct(moveDir, forward);
	const float rightDot   = FVector::DotProduct(moveDir, right);

	// Measure Direction Alignment
	// atan2(right, forward): 0 = forward, 90 = right, 180/-180 = back, -90 = left
	const float angleRad = FMath::Atan2(rightDot, forwardDot);
	float angleDeg = FMath::RadiansToDegrees(angleRad);

	// Normalize to [0, 360)
	if (angleDeg < 0.f) angleDeg += 360.f;

	// 8-way sectors, 45 degrees each
	if (angleDeg >= 337.5f || angleDeg < 22.5f) return EStickDirection::Forward;
	else if (angleDeg < 67.5f)   return EStickDirection::ForwardRight;
	else if (angleDeg < 112.5f)  return EStickDirection::Right;
	else if (angleDeg < 157.5f)  return EStickDirection::BackRight;
	else if (angleDeg < 202.5f)  return EStickDirection::Back;
	else if (angleDeg < 247.5f)  return EStickDirection::BackLeft;
	else if (angleDeg < 292.5f)  return EStickDirection::Left;
	else return EStickDirection::ForwardLeft;
}

EStickDirection UPlayerInputComponent::GetWorldDirRelativeToPlayerFacing(const FVector& WorldDir) const
{
	if (WorldDir.IsNearlyZero()) return EStickDirection::Neutral;

	FVector playerForward = player->GetActorForwardVector();
	playerForward.Z = 0.f;
	playerForward = playerForward.GetSafeNormal();

	FVector playerRight = player->GetActorRightVector();
	playerRight.Z = 0.f;
	playerRight = playerRight.GetSafeNormal();

	return GetStickDirFromWorldDir(WorldDir, playerForward, playerRight);
}

void UPlayerInputComponent::SetActionBuffer(const FGameplayTag& Action, const FVector2D& Move)
{
	if (!player) player = Cast<APlayer_Base>(GetOwner());
	if (!player) return;

	UWorld* world = GetWorld();
	if (!world) return;

	if (Action.MatchesTag(Tags::PlayerAction::AttackHeavyHold)) heavyStartTime = world->GetTimeSeconds();
	else if (Action.MatchesTag(Tags::PlayerAction::AttackLightHold)) lightStartTime = world->GetTimeSeconds();

	bufferedAction.time = world->GetTimeSeconds();
	bufferedAction.action = Action;
	bufferedAction.move = Move;

	SetComponentTickEnabled(true);
}

void UPlayerInputComponent::ClearActionBuffer()
{
	SetComponentTickEnabled(false);
	
	bufferedAction.time = -1.0f;
	bufferedAction.action = FGameplayTag::EmptyTag;
	bufferedAction.move = FVector2D::ZeroVector;
}

void UPlayerInputComponent::AddToMoveInputHistory(const FVector2D& Move)
{

}

bool UPlayerInputComponent::PerformedDirection(EStickDirection Direction, const FVector2D& Move) const
{
	if (!iCmbtInst) return false;

	switch (Direction)
	{
		case EStickDirection::Any:
			return true;
		
		case EStickDirection::Neutral:
			return Move.IsNearlyZero();
		
		case EStickDirection::NotNeutral:
			return !Move.IsNearlyZero();

		default:
			FVector localForward, localRight;
			FVector inputWorldDir = GetInputWorldDirRelativeToCamOrTarget(Move, localForward, localRight, iCmbtInst->GetCurrentTarget());
			EStickDirection lStickDir = GetStickDirFromWorldDir(inputWorldDir, localForward, localRight);
			return lStickDir == Direction;
	}
}

bool UPlayerInputComponent::PerformedMotion(EStickMotion Motion) const
{
    return false;
}

void UPlayerInputComponent::HandlePlayerInput(EPlayerInput PlayerInput, const FVector2D LookVector, const FVector2D MoveVector)
{
	if (!player) player = Cast<APlayer_Base>(GetOwner());
	if (!player) return;

	if (!stateMachineComp) player->FindComponentByClass<UStateMachineComponent>();
	if (!stateMachineComp) return;

	switch (PlayerInput)
	{
		case EPlayerInput::AttackHeavyTriggered:
		{
			if (UWorld* world = GetWorld())
			{
				heldTimeAtkHeavy = world->GetTimeSeconds() - heavyStartTime;
				bHeavyHeld = heldTimeAtkHeavy >= inputHeldThreshold;
			}
			if (!bHeavyHeld) return;
			PlayerInput = EPlayerInput::AttackHeavyOngoing;
			break;
		}

		case EPlayerInput::AttackHeavyStart:
			if (UWorld* world = GetWorld()) heavyStartTime = world->GetTimeSeconds();
			break;
		
		case EPlayerInput::AttackHeavyComplete:
			bHeavyHeld = false;
			if (UWorld* world = GetWorld()) heldTimeAtkHeavy = world->GetTimeSeconds() - heavyStartTime;
			break;

		case EPlayerInput::AttackLightTriggered:
		{
			if (UWorld* world = GetWorld())
			{
				heldTimeAtkLight = world->GetTimeSeconds() - lightStartTime;
				bLightHeld = heldTimeAtkLight >= inputHeldThreshold;
			}
			if (!bLightHeld) return;
			PlayerInput = EPlayerInput::AttackLightOngoing;
			break;
		}

		case EPlayerInput::AttackLightStart:
			if (UWorld* world = GetWorld()) lightStartTime = world->GetTimeSeconds();
			break;
		
		case EPlayerInput::AttackLightComplete:
			bLightHeld = false;
			if (UWorld* world = GetWorld()) heldTimeAtkLight = world->GetTimeSeconds() - lightStartTime;
			break;

		case EPlayerInput::BlockComplete:
			ClearActionBuffer();
			break;
		
		default:
			break;
	}

	const FGameplayTag playerAction = stateMachineComp->ResolvePlayerInput(PlayerInput, LookVector, MoveVector);
	player->TryAction(playerAction, LookVector, MoveVector);
}