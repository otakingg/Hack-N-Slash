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
const FGameplayTagContainer& AEnemyBase::GetTags() const { return gameplayTags; }

void AEnemyBase::AddTag(const FGameplayTag &Tag)
{
	if (!Tag.IsValid() || gameplayTags.HasTagExact(Tag)) return;
	gameplayTags.AddTag(Tag);
	OnTagsUpdated.Broadcast();
}

void AEnemyBase::RemoveTag(const FGameplayTag& Tag)
{
	if (!Tag.IsValid() || !gameplayTags.HasTagExact(Tag)) return;
	gameplayTags.RemoveTag(Tag);
	OnTagsUpdated.Broadcast();
}

bool AEnemyBase::HasTag(const FGameplayTag& Tag, bool bExact) const
{
    return Tag.IsValid() && (bExact ? gameplayTags.HasTagExact(Tag) : gameplayTags.HasTag(Tag));
}

bool AEnemyBase::HasAnyTag(const TArray<FGameplayTag>& TagArray, bool bExact) const
{
	if (TagArray.Num() == 0) return false;

	for (const FGameplayTag& Tag : TagArray)
	{
		if (Tag.IsValid() && (bExact ? gameplayTags.HasTagExact(Tag) : gameplayTags.HasTag(Tag))) return true;
	}
    return false;
}

bool AEnemyBase::HasAllTags(const TArray<FGameplayTag>& TagArray, bool bExact) const
{
	if (TagArray.Num() == 0) return false;

	for (const FGameplayTag& Tag : TagArray)
	{
		if (!Tag.IsValid() || !(bExact ? gameplayTags.HasTagExact(Tag) : gameplayTags.HasTag(Tag))) return false;
	}
    return true;
}

bool AEnemyBase::IsAirborne() const
{
    if (stateMachineComp) return HasTag(airborneTag);
	else if (moveComp) return moveComp->IsFalling();
	else return false;
}

bool AEnemyBase::IsGrounded() const
{
    if (stateMachineComp) return HasTag(groundedTag);
	else if (moveComp) return moveComp->IsMovingOnGround() ;
	else return false;
}

AActor* AEnemyBase::GetCurrentTarget() const { return brainComp ? brainComp->blackboard.TargetActor : nullptr; }
bool AEnemyBase::GetLockedOn() const { return brainComp ? brainComp->blackboard.bLockedOn : false; }

/************************************ Damageable Interface Functions ********************************/
void AEnemyBase::AttackDetected(const FAtkData& AtkData) { if (brainComp) brainComp->HandleAttackDetected(AtkData); }

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
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, FString::Printf(TEXT("[%s] Received Hit from %s"), *GetName(), *HitData.attacker->GetName()));
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

	// --- Resolve Poise ---
	if (bHasCombatRes) combatResComp->RecieveHit(HitData);
	
	// --- Apply Damage ---
	if (bHasStats)
	{
		HitData.dmgHPDealt = statsComp->ApplyDamage(HitData.dmgHP, HitData.penetration);
		if (!IsAlive())
		{
			if (!player) player = Cast<APlayer_Base>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
			if (player) player->HandleActorDeath(this);
		}
	}

	// --- State Machine ---
	const bool bHasReaction = HitData.resolvedReaction != Tags::StateMachine::Action::None;
	if (bHasReaction && bHasStateMachine) stateMachineComp->HandleReceiveHit(HitData);

	// --- AI Brain Post Hit ---
	if (bHasBrainComp) brainComp->HandleReceiveHitPost(HitData);

	if (bDebug)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, FString::Printf(TEXT("[%s] HitData.resolvedReaction = %s"), *GetName(), *HitData.resolvedReaction.ToString()));
		UE_LOG(LogTemp, Display, TEXT("[%s] HitData.resolvedReaction = %s"), *GetName(), *HitData.resolvedReaction.ToString());
	}

	OnHit.Broadcast(HitData);
}