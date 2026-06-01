#include "EnemyBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

#include "../Tags/CharacterStateTagNamespaces.h"
#include "../Combat/Shared/CombatResolutionComponent.h"
#include "../Combat/Shared/CombatTraceComponent.h"
#include "EnemyBrainComponent.h"
#include "../../Combat/Enemy/EnemyCombatComponent.h"
#include "../Shared/LocomotionComponent.h"
#include "../Player/Player_Base.h"
#include "../Shared/StateMachineComponent.h"
#include "../Shared/StatsComponent.h"

AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;
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
	
	UCharacterMovementComponent* moveComp = GetCharacterMovement();
	if (moveComp)
	{
		moveComp->bOrientRotationToMovement = false;
		moveComp->bUseControllerDesiredRotation = false;
	}
}

void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!player) player = Cast<APlayer_Base>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (player) SetActorTickEnabled(false);
}

void AEnemyBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

/************************************ Combat Interface Functions *************************************/
AActor* AEnemyBase::GetCurrentTarget() const { return brainComp ? brainComp->blackboard.TargetActor : nullptr; }

void AEnemyBase::Countered(AActor* Counteror, const FString& Reason)
{
	if (!stateMachineComp || !brainComp) return;

	if (bDebug)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, FString::Printf(TEXT("[%s] Countered! Reason: %s"), *GetName(), *Reason));
		UE_LOG(LogTemp, Display, TEXT("[%s] Countered! Reason: %s"), *GetName(), *Reason);
	}

	brainComp->HandleCountered(Counteror, Reason);
	stateMachineComp->OnCountered(Counteror, Reason);
}

/************************************ Damageable Interface Functions ********************************/
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
	HitData.resolvedReaction = ActionTags::None;

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
			HitData.resolvedReaction = HitTags::Dead;
			if (!player) player = Cast<APlayer_Base>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
			if (player) player->HandleActorDeath(this);
		}
	}

	// --- AI Brain Post Hit ---
	if (bHasBrainComp) brainComp->HandleReceiveHitPost(HitData);

	// --- State Machine ---
	const bool bHasReaction = HitData.resolvedReaction != ActionTags::None;
	if (bHasReaction && bHasStateMachine) stateMachineComp->OnReceiveHit(HitData);

	if (bDebug)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, FString::Printf(TEXT("[%s] HitData.resolvedReaction = %s"), *GetName(), *HitData.resolvedReaction.ToString()));
		UE_LOG(LogTemp, Display, TEXT("[%s] HitData.resolvedReaction = %s"), *GetName(), *HitData.resolvedReaction.ToString());
	}
}