#include "EnemyBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

#include "../Tags/CharacterStateTagNamespaces.h"
#include "../Combat/Shared/CombatResolutionComponent.h"
#include "../Combat/Shared/CombatTraceComponent.h"
#include "EnemyBrainComponent.h"
#include "../Shared/LocomotionComponent.h"
#include "../Player/Player_Base.h"
#include "../Shared/StateMachineComponent.h"
#include "../Shared/StatsComponent.h"

AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;
	brainComp = CreateDefaultSubobject<UEnemyBrainComponent>(TEXT("Enemy Brain"));
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
int AEnemyBase::GetPowerLevel() const {return combatResComp ? combatResComp->powerLvl : 0;}
int AEnemyBase::GetPowerLevelMax() const {return combatResComp ? combatResComp->powerLvlMax : 3;}



/************************************ Damageable Interface Functions ********************************/
bool AEnemyBase::IsAlive() const { return statsComp ? statsComp->IsAlive() : false; }

void AEnemyBase::ReceiveHit(FAtkHitData& HitData)
{
	if (!IsAlive()) return;
	
	const bool bHasCombatRes = combatResComp != nullptr;
	const bool bHasStateMachine = stateMachineComp != nullptr;
	const bool bHasStats = statsComp != nullptr;

	if (bHasCombatRes) combatResComp->ResolveHit(HitData);

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

	// --- Handle Reaction ---
	const bool bHasReaction = HitData.resolvedReaction != ActionTags::None;
	if (bHasReaction && bHasStateMachine) stateMachineComp->OnReceiveHit(HitData);

	// --- Notify brain ---
	if (brainComp) brainComp->HandleReceiveHit(HitData);
}