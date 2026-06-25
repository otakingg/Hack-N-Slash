// Fill out your copyright notice in the Description page of Project Settings.
#include "Player_Base.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/SpringArmComponent.h"

#include "../Tags/CharacterStateTags.h"
#include "../Combat/Shared/CombatResolutionComponent.h"
#include "../Combat/Shared/CombatTraceComponent.h"
#include "../Shared/LocomotionComponent.h"
#include "PlayerCamComponent.h"
#include "../Combat/Player/PlayerCombatCancelComponent.h"
#include "../Combat/Player/PlayerCombatComponent.h"
#include "../../Combat/Player/PlayerTargettingComponent.h"
#include "../../Characters/Shared/StateMachineComponent.h"
#include "../../Characters/Shared/StatsComponent.h"

APlayer_Base::APlayer_Base()
{
	PrimaryActorTick.bCanEverTick = false;

	combatComp = CreateDefaultSubobject<UPlayerCombatComponent>(TEXT("Combat"));
	combatResComp = CreateDefaultSubobject<UCombatResolutionComponent>(TEXT("Combat Resolution"));
	combatTraceComp = CreateDefaultSubobject<UCombatTraceComponent>(TEXT("Combat Trace"));
	locoComp = CreateDefaultSubobject<ULocomotionComponent>(TEXT("Locomotion"));
	playerCamComp = CreateDefaultSubobject<UPlayerCamComponent>(TEXT("Player Camera"));
	playerCombatCancelComp = CreateDefaultSubobject<UPlayerCombatCancelComponent>(TEXT("Player Combat Cancel"));
	playerTargettingComp = CreateDefaultSubobject<UPlayerTargettingComponent>(TEXT("Player Targetting"));
	stateMachineComp = CreateDefaultSubobject<UStateMachineComponent>(TEXT("State Machine"));
	statsComp = CreateDefaultSubobject<UStatsComponent>(TEXT("Stats"));
}

void APlayer_Base::BeginPlay()
{
	Super::BeginPlay();

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	UCharacterMovementComponent* moveComp = GetCharacterMovement();
	if (moveComp)
	{
		moveComp->bOrientRotationToMovement = true;
		moveComp->bUseControllerDesiredRotation = false;
	}

	UCameraComponent* camComp = FindComponentByClass<UCameraComponent>();
	if (camComp) camComp->bUsePawnControlRotation = false;
	
	USpringArmComponent* springArmComp = FindComponentByClass<USpringArmComponent>();
	if (!springArmComp) return;
	springArmComp->bDoCollisionTest = true;
	springArmComp->bUsePawnControlRotation = true;
}

void APlayer_Base::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlayer_Base::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* world = GetWorld()) world->GetTimerManager().ClearAllTimersForObject(this);
	Super::EndPlay(EndPlayReason);
}

// Called to bind functionality to input
void APlayer_Base::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void APlayer_Base::PlayerInput(EPlayerInput PlayerInput, const FVector2D& InputVector)
{
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

	const FGameplayTag CharacterActionTag = stateMachineComp->ResolvePlayerInput(PlayerInput, InputVector);

	if (CharacterActionTag.MatchesTag(CharacterActionTags::Attack) && combatComp) combatComp->Attack(CharacterActionTag, InputVector);
	else if (CharacterActionTag.MatchesTagExact(CharacterActionTags::BlockStart) && combatComp) combatComp->BlockStart();
	else if (CharacterActionTag.MatchesTagExact(CharacterActionTags::BlockRelease) && combatComp) combatComp->BlockStop();
	else if (CharacterActionTag.MatchesTagExact(CharacterActionTags::Dodge) && combatComp) combatComp->Dodge(InputVector);
	else if (CharacterActionTag.MatchesTagExact(CharacterActionTags::JumpStart) && locoComp) locoComp->JumpStart();
	else if (CharacterActionTag.MatchesTagExact(CharacterActionTags::JumpRelease) && locoComp) locoComp->JumpStop();
	else if (CharacterActionTag.MatchesTagExact(CharacterActionTags::LookMouse) && playerCamComp) playerCamComp->AddLookMouseInput(InputVector);
	else if (CharacterActionTag.MatchesTagExact(CharacterActionTags::LookStick) && playerCamComp) playerCamComp->AddLookStickInput(InputVector);
	else if (CharacterActionTag.MatchesTagExact(CharacterActionTags::Move) && locoComp) locoComp->Move(InputVector);
}

void APlayer_Base::HandleActorDeath(AActor* Actor)
{
	if (playerTargettingComp && playerTargettingComp->GetCurrentTarget() == Actor) playerTargettingComp->LockOff();
}

/************************************ Combat Interface Functions *************************************/
AActor* APlayer_Base::GetCurrentTarget() const { return playerTargettingComp ? playerTargettingComp->GetCurrentTarget() : nullptr; }
bool APlayer_Base::GetLockedOn() const { return playerTargettingComp ? playerTargettingComp->GetLockedOn() : false; }

/************************************ Damageable Interface Functions ********************************/
bool APlayer_Base::IsAlive() const { return statsComp ? statsComp->IsAlive() : false; }

void APlayer_Base::ReceiveHit(FAtkHitData& HitData)
{
	if (!IsAlive()) return;

	if (bDebug)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, FString::Printf(TEXT("[%s] Received Hit from %s"), *GetName(), *HitData.attacker->GetName()));
		UE_LOG(LogTemp, Warning, TEXT("ReceiveHit CALLED on %s by %s"), *GetName(), *GetNameSafe(HitData.attacker));
	}
	
	const bool bHasCombatRes = combatResComp != nullptr;
	const bool bHasCombatComp = combatComp != nullptr;
	const bool bHasStateMachine = stateMachineComp != nullptr;
	const bool bHasStats = statsComp != nullptr;
	HitData.resolvedReaction = StateActionTags::None;

	// --- Resolve Blocking ---
	if (bHasCombatComp) combatComp->ReceieveHit(HitData);

	// --- Resolve Poise ---
	if (bHasCombatRes) combatResComp->RecieveHit(HitData);

	// --- Apply Damage ---
	if (bHasStats) HitData.dmgHPDealt = statsComp->ApplyDamage(HitData.dmgHP, HitData.penetration);

	if (bDebug)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, FString::Printf(TEXT("[%s] HitData.resoleved reaction = %s"), *GetName(), *HitData.resolvedReaction.ToString()));
		UE_LOG(LogTemp, Display, TEXT("[%s] HitData.resolvedReaction = %s"), *GetName(), *HitData.resolvedReaction.ToString());
	}

	// --- State Machine ---
	const bool bHasReaction = HitData.resolvedReaction != StateActionTags::None;
	if (bHasReaction && bHasStateMachine) stateMachineComp->HandleReceiveHit(HitData);

	OnHit.Broadcast(HitData);
}