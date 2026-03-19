#include "EnemyBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../Combat/CombatResolutionComponent.h"
#include "../Combat/CombatTraceComponent.h"
#include "EnemyBrainComponent.h"
#include "EnemyLocomotionComponent.h"
#include "../StateMachineComponent.h"
#include "../StatsComponent.h"

namespace ReactionTags
{
    static const FGameplayTag None = FGameplayTag::RequestGameplayTag("State.Action.None");

    static const FGameplayTag Flinch = FGameplayTag::RequestGameplayTag("State.Action.Reaction.Flinch");

    static const FGameplayTag Stagger = FGameplayTag::RequestGameplayTag("State.Action.Reaction.Stagger");

    static const FGameplayTag Launch = FGameplayTag::RequestGameplayTag("State.Action.Reaction.Launch");

    static const FGameplayTag Knockback = FGameplayTag::RequestGameplayTag("State.Action.Reaction.Knockback");

    static const FGameplayTag Knockdown = FGameplayTag::RequestGameplayTag("State.Action.Reaction.Knockdown");
}

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
	if (statsComp->GetStat(EStat::Health) <= 0.0f || HitData.resolvedReaction == ReactionTags::None) return;
    else if (HitData.resolvedReaction == ReactionTags::Flinch && combatResComp) PlayFlinchAnim(HitData.hitDir);
	else stateMachineComp->OnReceiveHit(HitData);
}

void AEnemyBase::PlayFlinchAnim(FVector Direction)
{
	// Convert hit direction to local space
	/**
	 * X → Forward/Backward
	 * Y → Right/Left
	 * Z → Up/Down (usually ignored for hit reactions)
	 */
	FVector localHitDir = GetActorTransform().InverseTransformVectorNoScale(Direction);

	// Convert to angle (for BlendSpace or logic)
	/**
	 * 0° = front hit
	 * 180° or -180° = back hit
	 * 90° = right hit
	 * -90° = left hit
	 */
	float angle = FMath::Atan2(localHitDir.Y, localHitDir.X);
	angle = FMath::RadiansToDegrees(angle);

	FName sectionName;

	if (angle >= -45.f && angle <= 45.f) sectionName = "Front";
	else if (angle > 45.f && angle < 135.f) sectionName = "Right";
	else if (angle < -45.f && angle > -135.f) sectionName = "Left";
	else sectionName = "Back";

	combatResComp->PlayHitReaction(combatResComp->GetHitReactions().flinch, sectionName);
}