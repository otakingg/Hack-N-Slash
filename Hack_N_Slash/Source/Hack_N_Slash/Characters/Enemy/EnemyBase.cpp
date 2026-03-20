#include "EnemyBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../Tags/CharacterStateTagNamespaces.h"
#include "../Combat/CombatResolutionComponent.h"
#include "../Combat/CombatTraceComponent.h"
#include "EnemyBrainComponent.h"
#include "EnemyLocomotionComponent.h"
#include "../StateMachineComponent.h"
#include "../StatsComponent.h"

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
	//controller = GetController<AEnemyCrowdAIController>();
}

void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AEnemyBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

/************************************ Damageable Interface Functions ********************************/
void AEnemyBase::ReceiveHit(FAtkHitData& HitData)
{
	if (!statsComp) return;

	// Resolve Reaction
	if (combatResComp) combatResComp->ResolveHit(HitData);
	
	// Apply Damage
	statsComp->ApplyDamage(HitData.dmgHP, HitData.penetration);
	
	// Handle Reaction
	if (!stateMachineComp) return;

	// Stats comp broadcasts a death event. Death will be handled from that
	if (statsComp->GetStat(EStat::Health) <= 0.0f || HitData.resolvedReaction == HitTags::None) return;
    else if (HitData.resolvedReaction == HitTags::Flinch && combatResComp) PlayFlinchAnim(HitData);
	else stateMachineComp->OnReceiveHit(HitData);
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