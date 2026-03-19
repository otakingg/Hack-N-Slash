// Fill out your copyright notice in the Description page of Project Settings.
#include "Player_Base.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "../Combat/CombatResolutionComponent.h"
#include "../Combat/CombatTraceComponent.h"
#include "PlayerLocomotionComponent.h"
#include "../../Characters/StateMachineComponent.h"
#include "../../Characters/StatsComponent.h"

namespace ReactionTags
{
    static const FGameplayTag None = FGameplayTag::RequestGameplayTag("State.Action.None");

    static const FGameplayTag Flinch = FGameplayTag::RequestGameplayTag("State.Action.Reaction.Flinch");

    static const FGameplayTag Stagger = FGameplayTag::RequestGameplayTag("State.Action.Reaction.Stagger");

    static const FGameplayTag Launch = FGameplayTag::RequestGameplayTag("State.Action.Reaction.Launch");

    static const FGameplayTag Knockback = FGameplayTag::RequestGameplayTag("State.Action.Reaction.Knockback");

    static const FGameplayTag Knockdown = FGameplayTag::RequestGameplayTag("State.Action.Reaction.Knockdown");
}

APlayer_Base::APlayer_Base()
{
	PrimaryActorTick.bCanEverTick = false;

	combatResComp = CreateDefaultSubobject<UCombatResolutionComponent>(TEXT("Combat Resolution"));
	combatTraceComp = CreateDefaultSubobject<UCombatTraceComponent>(TEXT("Combat Trace"));
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
	if (IsValid(camComp)) camComp->bUsePawnControlRotation = false;
	
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

void APlayer_Base::PlayFlinchAnim(FVector Direction)
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