#include "Player_Base.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/SpringArmComponent.h"

#include "../Combat/Shared/CombatResolutionComponent.h"
#include "../Combat/Shared/CombatTraceComponent.h"
#include "../Structs/FAtkHitData.h"
#include "../Shared/LocomotionComponent.h"
#include "PlayerCamComponent.h"
#include "../Combat/Player/PlayerCombatCancelComponent.h"
#include "../Combat/Player/PlayerCombatComponent.h"
#include "../../Combat/Player/PlayerTargettingComponent.h"
#include "../../Characters/Shared/StateMachineComponent.h"
#include "../../Characters/Shared/StatsComponent.h"
#include "../Utility/Tags.h"

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

	moveComp = GetCharacterMovement();
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

void APlayer_Base::PlayerInput(EPlayerInput PlayerInput, const FVector2D LookVector, const FVector2D MoveVector)
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

	const FGameplayTag CharacterActionTag = stateMachineComp->ResolvePlayerInput(PlayerInput, LookVector, MoveVector);

	if (CharacterActionTag.MatchesTag(Tags::PlayerAction::Attack) && combatComp) combatComp->Attack(CharacterActionTag, MoveVector);
	else if (CharacterActionTag.MatchesTagExact(Tags::PlayerAction::BlockStart) && combatComp) combatComp->BlockStart();
	else if (CharacterActionTag.MatchesTagExact(Tags::PlayerAction::BlockHold) && combatComp) combatComp->BlockHold();
	else if (CharacterActionTag.MatchesTagExact(Tags::PlayerAction::BlockRelease) && combatComp) combatComp->BlockStop();
	else if (CharacterActionTag.MatchesTagExact(Tags::PlayerAction::Dodge) && combatComp) combatComp->Dodge(MoveVector);
	else if (CharacterActionTag.MatchesTagExact(Tags::PlayerAction::JumpStart) && locoComp) locoComp->JumpStart();
	else if (CharacterActionTag.MatchesTagExact(Tags::PlayerAction::JumpRelease) && locoComp) locoComp->JumpStop();
	else if (CharacterActionTag.MatchesTagExact(Tags::PlayerAction::LockOnOffStart) && playerTargettingComp) playerTargettingComp->ToggleLockOn();
	else if (CharacterActionTag.MatchesTagExact(Tags::PlayerAction::LookMouse) && playerCamComp) playerCamComp->AddLookMouseInput(LookVector);
	else if (CharacterActionTag.MatchesTagExact(Tags::PlayerAction::LookStick) && playerCamComp) playerCamComp->AddLookStickInput(LookVector);
	else if (CharacterActionTag.MatchesTagExact(Tags::PlayerAction::Move) && locoComp) locoComp->Move(MoveVector);
}

void APlayer_Base::HandleActorDeath(AActor* Actor)
{
	if (playerTargettingComp && playerTargettingComp->GetCurrentTarget() == Actor) playerTargettingComp->LockOff();
}

/************************************ Combat Interface Functions *************************************/
const FGameplayTagContainer& APlayer_Base::GetTags() const { return gameplayTags; }

void APlayer_Base::AddTag(const FGameplayTag& Tag)
{
	if (!Tag.IsValid() || gameplayTags.HasTagExact(Tag)) return;
	gameplayTags.AddTag(Tag);
	OnTagsUpdated.Broadcast();
}

void APlayer_Base::RemoveTag(const FGameplayTag& Tag)
{
	if (!Tag.IsValid() || !gameplayTags.HasTagExact(Tag)) return;
	gameplayTags.RemoveTag(Tag);
	OnTagsUpdated.Broadcast();
}

bool APlayer_Base::HasTag(const FGameplayTag& Tag, bool bExact) const
{
    return Tag.IsValid() && (bExact ? gameplayTags.HasTagExact(Tag) : gameplayTags.HasTag(Tag));
}

bool APlayer_Base::HasAnyTag(const TArray<FGameplayTag>& TagArray, bool bExact) const
{
	if (TagArray.Num() == 0) return false;

	for (const FGameplayTag& Tag : TagArray)
	{
		if (Tag.IsValid() && (bExact ? gameplayTags.HasTagExact(Tag) : gameplayTags.HasTag(Tag))) return true;
	}
    return false;
}

bool APlayer_Base::HasAllTags(const TArray<FGameplayTag>& TagArray, bool bExact) const
{
	if (TagArray.Num() == 0) return false;

	for (const FGameplayTag& Tag : TagArray)
	{
		if (!Tag.IsValid() || !(bExact ? gameplayTags.HasTagExact(Tag) : gameplayTags.HasTag(Tag))) return false;
	}
    return true;
}

bool APlayer_Base::IsAirborne() const
{
    if (stateMachineComp) return HasTag(airborneTag);
	else if (moveComp) return moveComp->IsFalling();
	else return false;
}

bool APlayer_Base::IsGrounded() const
{
    if (stateMachineComp) return HasTag(groundedTag);
	else if (moveComp) return moveComp->IsMovingOnGround() ;
	else return false;
}

AActor* APlayer_Base::GetCurrentTarget() const { return playerTargettingComp ? playerTargettingComp->GetCurrentTarget() : nullptr; }
bool APlayer_Base::GetLockedOn() const { return playerTargettingComp ? playerTargettingComp->GetLockedOn() : false; }

/************************************ Damageable Interface Functions ********************************/
void APlayer_Base::Countered(AActor* Counteror, const FString& Reason)
{
	if (!stateMachineComp) return;

	if (bDebug)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, FString::Printf(TEXT("[%s] Countered! Reason: %s"), *GetName(), *Reason));
		UE_LOG(LogTemp, Display, TEXT("[%s] Countered! Reason: %s"), *GetName(), *Reason);
	}

	stateMachineComp->HandleCountered(Counteror, Reason);
}

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
	HitData.resolvedReaction = Tags::StateMachine::Action::None;

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
	const bool bHasReaction = HitData.resolvedReaction != Tags::StateMachine::Action::None || !IsAlive();
	if (bHasReaction && bHasStateMachine) stateMachineComp->HandleReceiveHit(HitData);

	OnHit.Broadcast(HitData);
}