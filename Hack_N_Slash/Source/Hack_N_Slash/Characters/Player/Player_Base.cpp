// Fill out your copyright notice in the Description page of Project Settings.
#include "Player_Base.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "../Combat/CombatResolutionComponent.h"
#include "PlayerLocomotionComponent.h"
#include "../../Characters/StateMachineComponent.h"
#include "../../Characters/StatsComponent.h"

APlayer_Base::APlayer_Base()
{
	PrimaryActorTick.bCanEverTick = false;

	combatResComp = CreateDefaultSubobject<UCombatResolutionComponent>(TEXT("Combat Resolution"));
	playerLocoComp = CreateDefaultSubobject<UPlayerLocomotionComponent>(TEXT("Locomotion"));
	stateMachineComp = CreateDefaultSubobject<UStateMachineComponent>(TEXT("State Machine"));
	statsComp = CreateDefaultSubobject<UStatsComponent>(TEXT("Stats"));
}

void APlayer_Base::BeginPlay()
{
	Super::BeginPlay();

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	moveComp = GetCharacterMovement();
	if (moveComp)
	{
		moveComp->bOrientRotationToMovement = true;
		moveComp->bUseControllerDesiredRotation = false;
	}

	camComp = FindComponentByClass<UCameraComponent>();
	if (IsValid(camComp)) {camComp->bUsePawnControlRotation = false;}
	
	springArmComp = FindComponentByClass<USpringArmComponent>();
	if (!IsValid(springArmComp)) return;
	springArmComp->bDoCollisionTest = true;
	springArmComp->bUsePawnControlRotation = true;
}

void APlayer_Base::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void APlayer_Base::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void APlayer_Base::Input_AttackHeavy(const FVector2D& InputVector)
{
	if (stateMachineComp) stateMachineComp->RequestAttack(InputVector);
}

void APlayer_Base::Input_AttackLight(const FVector2D& InputVector)
{
	if (stateMachineComp) stateMachineComp->RequestAttack(InputVector);
}

void APlayer_Base::Input_BlockDodge(const FVector2D& InputVector)
{
	if (!stateMachineComp) return;
	if (InputVector.IsNearlyZero()) stateMachineComp->RequestBlockStart();
	else stateMachineComp->RequestDodge(InputVector);
}

void APlayer_Base::Input_BlockReleased()
{
	if (stateMachineComp) stateMachineComp->RequestBlockStop();
}

void APlayer_Base::Input_JumpPressed()
{
	if (stateMachineComp) stateMachineComp->RequestJumpPressed();
}

void APlayer_Base::Input_JumpReleased()
{
	if (stateMachineComp) stateMachineComp->RequestJumpReleased();
}

void APlayer_Base::Input_Look(const FVector2D &InputVector)
{
	if (stateMachineComp) stateMachineComp->RequestLook(InputVector);
}

void APlayer_Base::Input_Move(const FVector2D& InputVector)
{
	//In the future check if the player is blocking, and if so perform a dodge
	if (stateMachineComp) stateMachineComp->RequestMove(InputVector);
}

/************************************ Damageable Interface Functions ********************************/
void APlayer_Base::ReceiveHit(FAtkHitData& HitData)
{
	if (!statsComp || !combatResComp) return;

	// Resolve Reaction
	combatResComp->ResolveHit(HitData);
	
	// Apply Damage
	statsComp->ApplyDamage(HitData.dmgHP, HitData.penetration);
	
	// Handle Reaction
	if (!stateMachineComp) return;

	if (statsComp->GetStat(EStat::Health) <= 0.0f)
	{
		HitData.resolvedReaction = FGameplayTag::RequestGameplayTag(FName("State.Action.Reaction.Death"));
		if (UActionState* state = stateMachineComp->GetActionStateByTag(HitData.resolvedReaction)) stateMachineComp->ChangeActionState(state, false);
		// Make sure in the death state to play a falling death animation if in the air
	}
    else if (HitData.resolvedReaction == FGameplayTag::RequestGameplayTag(FName("State.Action.Reaction.Flinch"))) PlayAdditiveFlinch(HitData.hitDir);
	else if (HitData.resolvedReaction.IsValid())
    {
        if (UActionState* state = stateMachineComp->GetActionStateByTag(HitData.resolvedReaction)) stateMachineComp->ChangeActionState(state, false);
    }
	
}

void APlayer_Base::PlayAdditiveFlinch(FVector Direction)
{

}