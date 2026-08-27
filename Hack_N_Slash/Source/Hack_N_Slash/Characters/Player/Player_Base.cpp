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
#include "../Combat/Player/PlayerCombatComponent.h"
#include "PlayerInputComponent.h"
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
	inputComp = CreateDefaultSubobject<UPlayerInputComponent>(TEXT("Player Input"));
	locoComp = CreateDefaultSubobject<ULocomotionComponent>(TEXT("Locomotion"));
	playerCamComp = CreateDefaultSubobject<UPlayerCamComponent>(TEXT("Player Camera"));
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

void APlayer_Base::Tick(float DeltaTime) { Super::Tick(DeltaTime); }

void APlayer_Base::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* world = GetWorld()) world->GetTimerManager().ClearAllTimersForObject(this);
	Super::EndPlay(EndPlayReason);
}

// Called to bind functionality to input
void APlayer_Base::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) { Super::SetupPlayerInputComponent(PlayerInputComponent); }

void APlayer_Base::TryAction(const FGameplayTag& Action, const FVector2D Look, const FVector2D Move)
{
	if (Action.MatchesTag(Tags::PlayerAction::Attack) && combatComp) combatComp->Attack(Action, Move);
	else if (Action.MatchesTagExact(Tags::PlayerAction::BlockStart) && combatComp) combatComp->BlockStart();
	else if (Action.MatchesTagExact(Tags::PlayerAction::BlockHold) && combatComp) combatComp->BlockHold();
	else if (Action.MatchesTagExact(Tags::PlayerAction::BlockRelease) && combatComp) combatComp->BlockStop();
	else if (Action.MatchesTagExact(Tags::PlayerAction::Dodge) && combatComp) combatComp->Dodge(Move);
	else if (Action.MatchesTagExact(Tags::PlayerAction::JumpStart) && locoComp) locoComp->JumpStart();
	else if (Action.MatchesTagExact(Tags::PlayerAction::JumpRelease) && locoComp) locoComp->JumpStop();
	else if (Action.MatchesTagExact(Tags::PlayerAction::LockOnOffStart) && playerTargettingComp) playerTargettingComp->ToggleLockOn();
	else if (Action.MatchesTagExact(Tags::PlayerAction::LookMouse) && playerCamComp) playerCamComp->AddLookMouseInput(Look);
	else if (Action.MatchesTagExact(Tags::PlayerAction::LookStick) && playerCamComp) playerCamComp->AddLookStickInput(Look);
	else if (Action.MatchesTagExact(Tags::PlayerAction::Move))
	{
		if (inputComp) inputComp->AddToMoveInputHistory(Move);
		if (locoComp) locoComp->Move(Move);
	}
}

void APlayer_Base::TryBufferedAction(const FGameplayTag& Action, const FVector2D Move)
{
	if (Action.MatchesTag(Tags::PlayerAction::Attack) && combatComp) combatComp->Attack(Action, Move, true);
	else if (Action.MatchesTagExact(Tags::PlayerAction::BlockStart) && combatComp) combatComp->BlockStart(true);
	else if (Action.MatchesTagExact(Tags::PlayerAction::BlockHold) && combatComp) combatComp->BlockHold(true);
	else if (Action.MatchesTagExact(Tags::PlayerAction::Dodge) && combatComp) combatComp->Dodge(Move, true);
	else if (Action.MatchesTagExact(Tags::PlayerAction::JumpStart) && locoComp) locoComp->JumpStart(true);
}

void APlayer_Base::HandleActorDeath(AActor* Actor)
{
	if (playerTargettingComp && playerTargettingComp->GetCurrentTarget() == Actor) playerTargettingComp->LockOff();
}

/************************************ Combat Interface Functions *************************************/
//const FGameplayTagContainer& APlayer_Base::GetTags() const { return gameplayTags; }
const TMap<FGameplayTag, int32>& APlayer_Base::GetTags() const { return gameplayTags; }

void APlayer_Base::AddTag(const FGameplayTag& Tag)
{
	/*if (!Tag.IsValid() || gameplayTags.HasTagExact(Tag)) return;
	gameplayTags.AddTag(Tag);*/

	if (!Tag.IsValid()) return;

	int32& count = gameplayTags.FindOrAdd(Tag);
	++count;

	OnTagsUpdated.Broadcast();
}

void APlayer_Base::RemoveTag(const FGameplayTag& Tag)
{
	/*if (!Tag.IsValid() || !gameplayTags.HasTagExact(Tag)) return;
	gameplayTags.RemoveTag(Tag);*/

	if (!Tag.IsValid()) return;

	int32* count = gameplayTags.Find(Tag);
	if (!count) return;

	--(*count);
	if (*count <= 0) gameplayTags.Remove(Tag);


	OnTagsUpdated.Broadcast();
}

bool APlayer_Base::HasTag(const FGameplayTag& Tag, bool bExact) const
{
    //return Tag.IsValid() && (bExact ? gameplayTags.HasTagExact(Tag) : gameplayTags.HasTag(Tag));

	if (!Tag.IsValid()) return false;

	for (const TPair<FGameplayTag, int32>& pair : gameplayTags)
	{
		// Ignore tags with no active count
		if (pair.Value <= 0) continue;

		if (bExact && pair.Key.MatchesTagExact(Tag)) return true;
		else if (pair.Key.MatchesTag(Tag)) return true;
	}

	return false;
}

bool APlayer_Base::HasAnyTag(const TArray<FGameplayTag>& TagArray, bool bExact) const
{
	if (TagArray.Num() == 0) return false;

	/*for (const FGameplayTag& Tag : TagArray)
	{
		if (HasTag(Tag, bExact)) return true;
	}
    return false;*/

	for (const FGameplayTag& tag : TagArray)
	{
		if (HasTag(tag, bExact)) return true;
	}

	return false;
}

bool APlayer_Base::HasAllTags(const TArray<FGameplayTag>& TagArray, bool bExact) const
{
	if (TagArray.Num() == 0) return false;

	/*for (const FGameplayTag& Tag : TagArray)
	{
		if (!HasTag(Tag, bExact)) return false;
	}
    return true;*/

	for (const FGameplayTag& tag : TagArray)
	{
		if (!HasTag(tag, bExact)) return false;
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
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, FString::Printf(TEXT("[%s] Received Hit from %s"), *GetName(), *GetNameSafe(HitData.attacker)));
		UE_LOG(LogTemp, Warning, TEXT("ReceiveHit CALLED on %s by %s"), *GetName(), *GetNameSafe(HitData.attacker));
	}
	
	const bool bHasCombatRes = combatResComp != nullptr;
	const bool bHasCombatComp = combatComp != nullptr;
	const bool bHasStateMachine = stateMachineComp != nullptr;
	const bool bHasStats = statsComp != nullptr;
	HitData.resolvedReaction = Tags::StateMachine::Action::None;

	// --- Resolve Blocking ---
	if (bHasCombatComp) combatComp->ReceieveHit(HitData);


	// --- Apply Damage ---
	if (bHasStats) HitData.dmgDealt = statsComp->ApplyDamage(HitData.dmg, HitData.penetration);
	if (!IsAlive()) HitData.resolvedReaction = Tags::StateMachine::Action::Reaction::Dead;
	
	// --- Resolve Poise ---
	if (bHasCombatRes) combatResComp->RecieveHit(HitData);

	if (bDebug)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, FString::Printf(TEXT("[%s] HitData.resoleved reaction = %s"), *GetName(), *HitData.resolvedReaction.ToString()));
		UE_LOG(LogTemp, Display, TEXT("[%s] HitData.resolvedReaction = %s"), *GetName(), *HitData.resolvedReaction.ToString());
	}

	// --- State Machine ---
	if (bHasStateMachine) stateMachineComp->HandleReceiveHit(HitData);

	OnHit.Broadcast(HitData);
}