#include "EnemyBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "EnemyBrainComponent.h"
#include "EnemyLocomotionComponent.h"
#include "../StateMachineComponent.h"
#include "../StatsComponent.h"

AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = false;
	brainComp = CreateDefaultSubobject<UEnemyBrainComponent>(TEXT("Enemy Brain"));
	enemyLocomotionComp = CreateDefaultSubobject<UEnemyLocomotionComponent>(TEXT("Enemy Locomotion"));
	stateMachineComp = CreateDefaultSubobject<UStateMachineComponent>(TEXT("State Machine"));
	statsComp = CreateDefaultSubobject<UStatsComponent>(TEXT("Stats"));
}

// Called when the game starts or when spawned
void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	moveComp = GetCharacterMovement();
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