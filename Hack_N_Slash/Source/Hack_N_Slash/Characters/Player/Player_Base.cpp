// Fill out your copyright notice in the Description page of Project Settings.
#include "Player_Base.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "../Tags/CharacterStateTagNamespaces.h"
#include "../Combat/CombatResolutionComponent.h"
#include "../Combat/CombatTraceComponent.h"
#include "PlayerCamComponent.h"
#include "PlayerLocomotionComponent.h"
#include "../../Characters/StateMachineComponent.h"
#include "../../Characters/StatsComponent.h"

APlayer_Base::APlayer_Base()
{
	PrimaryActorTick.bCanEverTick = false;

	combatResComp = CreateDefaultSubobject<UCombatResolutionComponent>(TEXT("Combat Resolution"));
	combatTraceComp = CreateDefaultSubobject<UCombatTraceComponent>(TEXT("Combat Trace"));
	playerCamComp = CreateDefaultSubobject<UPlayerCamComponent>(TEXT("Player Camera"));
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

void APlayer_Base::Input_BlockDodgeReleased()
{
	if (stateMachineComp && stateMachineComp->HasExactActiveTag(CombatTags::Block)) stateMachineComp->RequestBlockStop();
}

void APlayer_Base::Input_JumpPressed()
{
	if (stateMachineComp) stateMachineComp->RequestJumpPressed();
}

void APlayer_Base::Input_JumpReleased()
{
	if (stateMachineComp) stateMachineComp->RequestJumpReleased();
}

void APlayer_Base::Input_Look(const FVector2D& InputVector)
{
	if (stateMachineComp) stateMachineComp->RequestLook(InputVector);
}

void APlayer_Base::Input_Move(const FVector2D& InputVector)
{
	if (stateMachineComp) stateMachineComp->RequestMove(InputVector);
}

void APlayer_Base::Input_MoveStart(const FVector2D& InputVector)
{
	if (!stateMachineComp) return;
	else if (stateMachineComp->HasExactActiveTag(CombatTags::Block) && !InputVector.IsNearlyZero()) stateMachineComp->RequestDodge(InputVector);
	else stateMachineComp->RequestMove(InputVector);
}

void APlayer_Base::PlayFlinchAnim(const FAtkHitData &HitData)
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
/************************************ Combat Interface Functions *************************************/
int APlayer_Base::GetPowerLevel() const {return combatResComp ? combatResComp->powerLvl : 0;}
int APlayer_Base::GetPowerLevelMax() const {return combatResComp ? combatResComp->powerLvlMax : 3;}



/************************************ Damageable Interface Functions ********************************/
bool APlayer_Base::IsAlive() const { return statsComp ? statsComp->IsAlive() : false; }

void APlayer_Base::ReceiveHit(FAtkHitData& HitData)
{
	if (!IsAlive()) return;
	
	const bool bHasCombatRes = combatResComp != nullptr;
	const bool bHasStateMachine = stateMachineComp != nullptr;
	const bool bHasStats = statsComp != nullptr;

	// --- Resolve Reaction (optional) ---
	if (bHasCombatRes) combatResComp->ResolveHit(HitData);

	// --- Apply Damage (optional) ---
	if (bHasStats)
	{
		HitData.dmgHPDealt = statsComp->ApplyDamage(HitData.dmgHP, HitData.penetration);
		if (!IsAlive()) HitData.resolvedReaction = HitTags::Dead;
	}

	// --- Handle Reaction / State Machine (optional) ---
	const bool bHasReaction = HitData.resolvedReaction != ActionTags::None;

	if (bHasReaction && bHasCombatRes)
	{
		if (HitData.resolvedReaction == HitTags::Flinch) PlayFlinchAnim(HitData);
		else if (bHasStateMachine) stateMachineComp->OnReceiveHit(HitData);
	}
}