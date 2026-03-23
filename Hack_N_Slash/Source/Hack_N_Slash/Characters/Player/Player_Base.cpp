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
	if (statsComp->GetStat(EStat::Health) <= 0.0f || HitData.resolvedReaction == HitTags::None) return;
    else if (HitData.resolvedReaction == HitTags::Flinch && combatResComp) PlayFlinchAnim(HitData);
	else stateMachineComp->OnReceiveHit(HitData);
}

void APlayer_Base::PlayFlinchAnim(const FAtkHitData& HitData)
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