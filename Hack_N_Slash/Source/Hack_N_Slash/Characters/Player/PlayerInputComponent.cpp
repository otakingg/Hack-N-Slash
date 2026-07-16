#include "PlayerInputComponent.h"
#include "Player_Base.h"
#include "../Shared/StateMachineComponent.h"

UPlayerInputComponent::UPlayerInputComponent() { PrimaryComponentTick.bCanEverTick = true; }

void UPlayerInputComponent::BeginPlay()
{
	Super::BeginPlay();

	player = Cast<APlayer_Base>(GetOwner());
	stateMachineComp = player ? player->FindComponentByClass<UStateMachineComponent>() : nullptr;
}

void UPlayerInputComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bufferedAction.time < 0.0f)
	{
		ClearActionBuffer();
		SetComponentTickEnabled(false);
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
		
		default:
			break;
	}

	const FGameplayTag playerAction = stateMachineComp->ResolvePlayerInput(PlayerInput, LookVector, MoveVector);
	player->TryAction(playerAction, LookVector, MoveVector);
}

void UPlayerInputComponent::SetActionBuffer(const FGameplayTag& Action, const FVector2D& Move)
{
	if (!player) player = Cast<APlayer_Base>(GetOwner());
	if (!player) return;

	UWorld* world = GetWorld();
	if (!world) return;

	bufferedAction.time = world->GetTimeSeconds();
	bufferedAction.action = Action;
	bufferedAction.move = Move;

	SetComponentTickEnabled(true);
}

void UPlayerInputComponent::ClearActionBuffer()
{
	bufferedAction.time = -1.0f;
	bufferedAction.action = FGameplayTag::EmptyTag;
	bufferedAction.move = FVector2D::ZeroVector;
}