#include "PlayerInputComponent.h"
#include "../../Interfaces/CombatInstigator.h"
#include "Player_Base.h"
#include "../Shared/StateMachineComponent.h"

int32 UPlayerInputComponent::DirectionToIndex(EStickDirection Direction)
{
    switch (Direction)
    {
        case EStickDirection::Forward:      return 0;
        case EStickDirection::ForwardRight: return 1;
        case EStickDirection::Right:        return 2;
        case EStickDirection::BackRight:    return 3;
        case EStickDirection::Back:         return 4;
        case EStickDirection::BackLeft:     return 5;
        case EStickDirection::Left:         return 6;
        case EStickDirection::ForwardLeft:  return 7;

        default:
            return -1;
    }
}

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

	if (bufferedAction.time < 0.0f) // Fail-safe
	{
		ClearActionBuffer();
		return;
	}

	UWorld* world = GetWorld();
	if (!world) return;

	// Try the buffered input action, if still within the buffer time frame. Clear the buffer if time has expired
	float timeSinceInputAction = world->GetTimeSeconds() - bufferedAction.time;
	if (timeSinceInputAction > actionBufferMaxTime) ClearActionBuffer();
	else if (player) player->TryBufferedAction(bufferedAction.action, bufferedAction.move);
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
	SetComponentTickEnabled(false); // Optimization
	
	bufferedAction.time = -1.0f;
	bufferedAction.action = FGameplayTag::EmptyTag;
	bufferedAction.move = FVector2D::ZeroVector;
}

void UPlayerInputComponent::AddToMoveInputHistory(const FVector2D& Move)
{
	UWorld* world = GetWorld();
	if (!world || !iCmbtInst || Move.IsNearlyZero()) return;

    // Convert the current stick position into an 8-way direction
    FVector localForward, localRight;
    FVector inputWorldDir = GetInputWorldDirRelativeToCamOrTarget(Move, localForward, localRight, iCmbtInst->GetCurrentTarget());
    const EStickDirection direction = GetStickDirFromWorldDir(inputWorldDir, localForward, localRight);

    // Don't add duplicates
    if (moveInputHistory.Num() > 0 && moveInputHistory.Last().direction == direction) return;

	float currentTime = world->GetTimeSeconds();

    // Remove expired entries
    while (moveInputHistory.Num() > 0 && currentTime - moveInputHistory[0].time > moveInputHistoryMaxTime) moveInputHistory.RemoveAt(0);

    // Add the new input
    FMoveInput newInput;
    newInput.time = currentTime;
    newInput.direction = direction;

    moveInputHistory.Add(newInput);

    // Keep the history bounded
    while (moveInputHistory.Num() > 16) moveInputHistory.RemoveAt(0);
}

bool UPlayerInputComponent::AreDirectionsAdjacent(EStickDirection DirectionA, EStickDirection DirectionB, int32 Tolerance) const
{
	// Tolerance = 1 means "within one sector"
	// EX: Back matches: Back, BackRight, BackLeft
    const int32 indexA = DirectionToIndex(DirectionA);
    const int32 indexB = DirectionToIndex(DirectionB);

    if (indexA < 0 || indexB < 0) return false;

    // Smallest distance around the circle
	// Absolute value + min accounts for clockiwse and counter-clockwise
    const int32 delta = FMath::Abs(indexA - indexB);
    const int32 circularDistance = FMath::Min(delta, 8 - delta);

    return circularDistance <= Tolerance;
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

		default:
			FVector localForward, localRight;
			FVector inputWorldDir = GetInputWorldDirRelativeToCamOrTarget(Move, localForward, localRight, iCmbtInst->GetCurrentTarget());
			EStickDirection lStickDir = GetStickDirFromWorldDir(inputWorldDir, localForward, localRight);
			return lStickDir == Direction;
	}
}

bool UPlayerInputComponent::PerformedMotion(EStickMotion Motion)
{
	UWorld* world = GetWorld();
	if (!world) return false;
	
	float currentTime = world->GetTimeSeconds();

    // Remove expired entries
    while (moveInputHistory.Num() > 0 && currentTime - moveInputHistory[0].time > moveInputHistoryMaxTime) moveInputHistory.RemoveAt(0);

	switch (Motion)
	{
		case EStickMotion::Circle:
			return PerformedCircle();

		case EStickMotion::BackForward:
			return PerformedLinearMotion(EStickDirection::Back, EStickDirection::Forward);

		case EStickMotion::ForwardBack:
			return PerformedLinearMotion(EStickDirection::Forward, EStickDirection::Back);

		case EStickMotion::LeftRight:
			return PerformedLinearMotion(EStickDirection::Left, EStickDirection::Right);

		case EStickMotion::RightLeft:
			return PerformedLinearMotion(EStickDirection::Right, EStickDirection::Left);
		
		default:
			return false;
	}
}

bool UPlayerInputComponent::PerformedCircle() const
{
	constexpr float sectorAngle = 45.0f;
	constexpr float requiredRotation = 270.0f; // 270 means it's ok to skip at most 45° of the circle

	float cwRotation = 0.0f;
	float ccWRotation = 0.0f;

	for (int32 i = 1; i < moveInputHistory.Num(); ++i)
	{
		int32 prev = DirectionToIndex(moveInputHistory[i - 1].direction);
		int32 curr = DirectionToIndex(moveInputHistory[i].direction);

		if (prev < 0 || curr < 0) continue;

		// A perfect clockwise movement has Delta = 1. A forgiving implementation has Delta = 2
		// A perfect counter-clockwise movement has Delta = 7. A forgiving implementation has Delta = 6
		int32 delta = (curr - prev + 8) % 8;

		// Clockwise
		if (delta == 1)
		{
			cwRotation += sectorAngle;
			ccWRotation = 0.f;
		}

		// Skipped one direction but still clockwise
		// Just don't add the sector angle because you skipped one
		else if (delta == 2) ccWRotation = 0.f;


		// Counter-clockwise
		else if (delta == 7)
		{
			ccWRotation += sectorAngle;
			cwRotation = 0.f;
		}
		// Skipped one direction but still counter-clockwise
		// Just don't add the sector angle because you skipped one
		else if (delta == 6) cwRotation = 0.f;


		// Direction changed randomly, reset
		else
		{
			cwRotation = 0.f;
			ccWRotation = 0.f;
		}


		if (cwRotation >= requiredRotation || ccWRotation >= requiredRotation) return true;
	}
	
	return false;
}

bool UPlayerInputComponent::PerformedLinearMotion(EStickDirection Start, EStickDirection End) const
{
    if (moveInputHistory.Num() < 2) return false;

    const EStickDirection prev = moveInputHistory[moveInputHistory.Num() - 2].direction;
    const EStickDirection curr = moveInputHistory.Last().direction;

    int32 startIndex = -1;

	for (int32 i = 0; i < moveInputHistory.Num(); ++i)
	{
		// if (AreDirectionsAdjacent(moveInputHistory[i].direction, Start, 1))
		if (moveInputHistory[i].direction == Start) // Must start eaxctly with the start direction
		{
			startIndex = i;
			break;
		}
	}

	if (startIndex < 0) return false;

	// At most 3 sectors can be covered
	int32 maxStep = 2;
	int32 stepCount = 0;

	for (int32 i = startIndex + 1; i < moveInputHistory.Num(); ++i)
	{
		++stepCount;
		if (AreDirectionsAdjacent(moveInputHistory[i].direction, End, 1)) break;
	}

    return stepCount <= maxStep;
}

void UPlayerInputComponent::HandlePlayerInput(EPlayerInput PlayerInput, const FVector2D LookVector, const FVector2D MoveVector)
{
	if (!player || !stateMachineComp) return;

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