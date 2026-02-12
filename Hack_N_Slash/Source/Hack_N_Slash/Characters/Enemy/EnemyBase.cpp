#include "EnemyBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Characters/StateMachineComponent.h"
#include "Characters/StatsComponent.h"

AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = false;
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