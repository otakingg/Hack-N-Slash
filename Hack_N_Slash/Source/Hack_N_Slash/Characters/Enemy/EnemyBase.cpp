#include "EnemyBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../Interfaces/CharAnimInterface.h"
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

	if (USkeletalMeshComponent* skeletalMeshComp = GetMesh())
	{
		iParentAnimInst = Cast<ICharAnimInterface>(skeletalMeshComp->GetAnimInstance());
		const TArray<USceneComponent*> children {skeletalMeshComp->GetAttachChildren()};
		if (!children.IsEmpty())
		{
			USkeletalMeshComponent* childSkeletalMeshComp {Cast<USkeletalMeshComponent>(children[0])};
			if (childSkeletalMeshComp) {iChildAnimInst = Cast<ICharAnimInterface>(childSkeletalMeshComp->GetAnimInstance());}
		}
	}
	
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
	if (!statsComp || !combatResComp) return;

	// Resolve Reaction
	combatResComp->ResolveHit(HitData);
	
	// Apply Damage
	statsComp->ApplyDamage(HitData.dmgHP, HitData.penetration);
	
	// Handle Reaction
	if (!stateMachineComp) return;

	// Stats broadcasts a death event. Death will be handled from that
	if (statsComp->GetStat(EStat::Health) <= 0.0f || HitData.resolvedReaction == FGameplayTag::RequestGameplayTag(FName("State.Action.None"))) return;
    else if (HitData.resolvedReaction == FGameplayTag::RequestGameplayTag(FName("State.Action.Reaction.Flinch"))) PlayAdditiveFlinch(HitData.hitDir);
	else if (HitData.resolvedReaction.IsValid())
    {
        if (UActionState* state = stateMachineComp->GetActionStateByTag(HitData.resolvedReaction)) stateMachineComp->ChangeActionState(state, false);
    }
	
}

void AEnemyBase::PlayAdditiveFlinch(FVector Direction)
{
	// Choose animation based on direction, then play it
	if (iParentAnimInst) iParentAnimInst->PlayMontageHNS();
}