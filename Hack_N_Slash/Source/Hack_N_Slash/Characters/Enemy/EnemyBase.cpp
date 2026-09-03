#include "EnemyBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

#include "../Combat/Shared/CombatResolutionComponent.h"
#include "../Combat/Shared/CombatTraceComponent.h"
#include "EnemyBrainComponent.h"
#include "../../Combat/Enemy/EnemyCombatComponent.h"
#include "../Structs/FAtkHitData.h"
#include "../Shared/LocomotionComponent.h"
#include "../Player/Player_Base.h"
#include "../Shared/StateMachineComponent.h"
#include "../Shared/StatsComponent.h"
#include "../Utility/Tags.h"

AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = false;
	brainComp = CreateDefaultSubobject<UEnemyBrainComponent>(TEXT("Enemy Brain"));
	combatComp = CreateDefaultSubobject<UEnemyCombatComponent>(TEXT("Combat"));
	combatResComp = CreateDefaultSubobject<UCombatResolutionComponent>(TEXT("Combat Resolution"));
	combatTraceComp = CreateDefaultSubobject<UCombatTraceComponent>(TEXT("Combat Trace"));
	locoComp = CreateDefaultSubobject<ULocomotionComponent>(TEXT("Locomotion"));
	stateMachineComp = CreateDefaultSubobject<UStateMachineComponent>(TEXT("State Machine"));
	statsComp = CreateDefaultSubobject<UStatsComponent>(TEXT("Stats"));
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = true;
	
	moveComp = GetCharacterMovement();
	if (moveComp)
	{
		moveComp->bOrientRotationToMovement = false;
		moveComp->bUseControllerDesiredRotation = false;
	}
}

void AEnemyBase::Tick(float DeltaTime) { Super::Tick(DeltaTime); }

void AEnemyBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) { Super::SetupPlayerInputComponent(PlayerInputComponent); }

/************************************ Combat Interface Functions *************************************/
//const FGameplayTagContainer& AEnemyBase::GetTags() const { return gameplayTags; }
const TMap<FGameplayTag, int32>& AEnemyBase::GetTags() const { return gameplayTags; }

void AEnemyBase::AddTag(const FGameplayTag &Tag)
{
	/*if (!Tag.IsValid() || gameplayTags.HasTagExact(Tag)) return;
	gameplayTags.AddTag(Tag);*/

	if (!Tag.IsValid()) return;

	int32& count = gameplayTags.FindOrAdd(Tag);
	++count;

	OnTagsUpdated.Broadcast();
}

void AEnemyBase::RemoveTag(const FGameplayTag& Tag)
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

bool AEnemyBase::HasTag(const FGameplayTag& Tag, bool bExact) const
{
    //return Tag.IsValid() && (bExact ? gameplayTags.HasTagExact(Tag) : gameplayTags.HasTag(Tag));

	if (!Tag.IsValid()) return false;

	for (const TPair<FGameplayTag, int32>& pair : gameplayTags)
	{
		// Ignore tags with no active count
		if (pair.Value <= 0) continue;

		if (bExact)
		{
			if (pair.Key.MatchesTagExact(Tag)) return true;
		}
		else if (pair.Key.MatchesTag(Tag)) return true;
	}

	return false;
}

bool AEnemyBase::HasAnyTag(const TArray<FGameplayTag>& TagArray, bool bExact) const
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

bool AEnemyBase::HasAllTags(const TArray<FGameplayTag>& TagArray, bool bExact) const
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

AActor* AEnemyBase::GetCurrentTarget() const { return brainComp ? brainComp->blackboard.TargetActor : nullptr; }
bool AEnemyBase::GetLockedOn() const { return brainComp ? brainComp->blackboard.bLockedOn : false; }

/************************************ Damageable Interface Functions ********************************/
void AEnemyBase::Countered(AActor* Counteror, const FString& Reason)
{
	if (!stateMachineComp || !brainComp) return;

	if (bDebug)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, FString::Printf(TEXT("[%s] Countered! Reason: %s"), *GetName(), *Reason));
		UE_LOG(LogTemp, Display, TEXT("[%s] Countered! Reason: %s"), *GetName(), *Reason);
	}

	stateMachineComp->HandleCountered(Counteror, Reason);
	brainComp->HandleCountered(Counteror, Reason);
}

bool AEnemyBase::IsAlive() const { return statsComp ? statsComp->IsAlive() : false; }

void AEnemyBase::ReceiveHit(FAtkHitData& HitData)
{
	if (!IsAlive()) return;

	if (bDebug)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, FString::Printf(TEXT("[%s] Received Hit from %s"), *GetName(), *GetNameSafe(HitData.attacker)));
		UE_LOG(LogTemp, Warning, TEXT("ReceiveHit CALLED on %s by %s"), *GetName(), *GetNameSafe(HitData.attacker));
	}

	const bool bHasBrainComp = brainComp != nullptr;
	const bool bHasCombatComp = combatComp != nullptr;
	const bool bHasCombatRes = combatResComp != nullptr;
	const bool bHasStateMachine = stateMachineComp != nullptr;
	const bool bHasStats = statsComp != nullptr;
	HitData.resolvedReaction = Tags::StateMachine::Action::None;

	// --- AI Brain Pre Hit ---
	if (bHasBrainComp) brainComp->HandleReceiveHitPre(HitData);

	// --- Custom Enemy Behavior ---
	if (bHasCombatComp) combatComp->ReceieveHit(HitData);

	// --- Apply Damage ---
	if (bHasStats)
	{
		HitData.dmgDealt = statsComp->ApplyDamage(HitData.dmg, HitData.penetration);
		if (!IsAlive())
		{
			HitData.resolvedReaction = Tags::StateMachine::Action::Reaction::Dead;
			if (!player) player = Cast<APlayer_Base>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
			if (player) player->HandleActorDeath(this);
		}
	}
	
	// --- Resolve Poise ---
	if (bHasCombatRes) combatResComp->RecieveHit(HitData);

	// --- State Machine ---
	if (bHasStateMachine) stateMachineComp->HandleReceiveHit(HitData);

	// --- AI Brain Post Hit ---
	if (bHasBrainComp) brainComp->HandleReceiveHitPost(HitData);

	if (bDebug)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, FString::Printf(TEXT("[%s] HitData.resolvedReaction = %s"), *GetName(), *HitData.resolvedReaction.ToString()));
		UE_LOG(LogTemp, Display, TEXT("[%s] HitData.resolvedReaction = %s"), *GetName(), *HitData.resolvedReaction.ToString());
	}

	OnHit.Broadcast(HitData);
}

/************************************ Enemy Interface Functions ********************************/
bool AEnemyBase::HasSuperArmor() const { return combatComp && combatComp->HasSuperArmor(); }