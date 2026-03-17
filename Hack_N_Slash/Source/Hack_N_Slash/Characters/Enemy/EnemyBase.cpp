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
    else if (HitData.resolvedReaction == ReactionTags::Flinch) PlayAdditiveFlinch(HitData.hitDir);
	else if (HitData.resolvedReaction.IsValid())
    {
        if (UActionState* state = stateMachineComp->GetActionStateByTag(HitData.resolvedReaction)) stateMachineComp->ChangeActionState(state, false);
    }
	
}

void AEnemyBase::PlayAdditiveFlinch(FVector Direction)
{
	// Choose animation based on direction, then play it
	combatResComp->PlayHitReaction();
}