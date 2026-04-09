#include "EnemyBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../Tags/CharacterStateTagNamespaces.h"
#include "../Combat/Shared/CombatResolutionComponent.h"
#include "../Combat/Shared/CombatTraceComponent.h"
#include "EnemyBrainComponent.h"
#include "EnemyLocomotionComponent.h"
#include "../Shared/StateMachineComponent.h"
#include "../Shared/StatsComponent.h"

AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = false;
	brainComp = CreateDefaultSubobject<UEnemyBrainComponent>(TEXT("Enemy Brain"));
	combatResComp = CreateDefaultSubobject<UCombatResolutionComponent>(TEXT("Combat Resolution"));
	combatTraceComp = CreateDefaultSubobject<UCombatTraceComponent>(TEXT("Combat Trace"));
	enemyLocomotionComp = CreateDefaultSubobject<UEnemyLocomotionComponent>(TEXT("Enemy Locomotion"));
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
		if (!IsAlive()) HitData.resolvedReaction = HitTags::Dead;
	}

	// --- Handle Reaction ---
	const bool bHasReaction = HitData.resolvedReaction != ActionTags::None;

	if (bHasReaction && bHasCombatRes)
	{
		if (HitData.resolvedReaction == HitTags::Flinch) PlayFlinchAnim(HitData);
		else if (bHasStateMachine) stateMachineComp->OnReceiveHit(HitData);
	}

	// --- Notify brain ---
	if (brainComp) brainComp->HandleReceiveHit(HitData);
}

void AEnemyBase::PlayFlinchAnim(const FAtkHitData& HitData)
{
    // Calculate hit direction
    FVector hitDir = FVector::ZeroVector;
    if (HitData.attacker) hitDir = (HitData.attacker->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    else hitDir = (HitData.hitLoc - GetActorLocation()).GetSafeNormal();
    
    // Flatten
    hitDir.Z = 0.f;
    hitDir.Normalize();

    // ✅ Use ONLY yaw rotation (ignores mesh weirdness)
    FRotator YawRot = GetActorRotation();
    YawRot.Pitch = 0.f;
    YawRot.Roll = 0.f;

    FVector Forward = YawRot.Vector(); // clean forward
    FVector Right   = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);

    float ForwardDot = FVector::DotProduct(hitDir, Forward);
    float RightDot   = FVector::DotProduct(hitDir, Right);

    float angle = FMath::RadiansToDegrees(FMath::Atan2(RightDot, ForwardDot));

	FName sectionName;

	if (angle >= -45.f && angle <= 45.f) sectionName = "Front";
	else if (angle > 45.f && angle < 135.f) sectionName = "Right";
	else if (angle < -45.f && angle > -135.f) sectionName = "Left";
	else sectionName = "Back";

	combatResComp->PlayHitReaction(combatResComp->GetHitReactions().flinch, sectionName);
}