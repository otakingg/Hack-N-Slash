// Fill out your copyright notice in the Description page of Project Settings.
#include "Player_Base.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/SpringArmComponent.h"

#include "../Tags/CharacterStateTagNamespaces.h"
#include "../Combat/Shared/CombatResolutionComponent.h"
#include "../Combat/Shared/CombatTraceComponent.h"
#include "../../Enums/EPlayerAction.h"
#include "../Shared/LocomotionComponent.h"
#include "PlayerCamComponent.h"
#include "../Combat/Player/PlayerCombatCancelComponent.h"
#include "../Combat/Player/PlayerCombatComponent.h"
#include "../../Combat/Player/PlayerTargettingComponent.h"
#include "../../Characters/Shared/StateMachineComponent.h"
#include "../../Characters/Shared/StatsComponent.h"

APlayer_Base::APlayer_Base()
{
	PrimaryActorTick.bCanEverTick = false;

	combatResComp = CreateDefaultSubobject<UCombatResolutionComponent>(TEXT("Combat Resolution"));
	combatTraceComp = CreateDefaultSubobject<UCombatTraceComponent>(TEXT("Combat Trace"));
	locoComp = CreateDefaultSubobject<ULocomotionComponent>(TEXT("Locomotion"));
	playerCamComp = CreateDefaultSubobject<UPlayerCamComponent>(TEXT("Player Camera"));
	playerCombatCancelComp = CreateDefaultSubobject<UPlayerCombatCancelComponent>(TEXT("Player Combat Cancel"));
	playerCombatComp = CreateDefaultSubobject<UPlayerCombatComponent>(TEXT("Player Combat"));
	playerTargettingComp = CreateDefaultSubobject<UPlayerTargettingComponent>(TEXT("Player Targetting"));
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

void APlayer_Base::Input_Started_AttackHeavy(const FVector2D& InputVector)
{
	if (stateMachineComp) stateMachineComp->RequestAttack(InputVector, EPlayerAction::AttackHeavyStart);
	else if (playerCamComp) playerCombatComp->AttackHeavyStart(InputVector);
}

void APlayer_Base::Input_Started_AttackLight(const FVector2D& InputVector)
{
	if (stateMachineComp) stateMachineComp->RequestAttack(InputVector, EPlayerAction::AttackLightStart);
	else if (playerCamComp) playerCombatComp->AttackLightStart(InputVector);
}

void APlayer_Base::Input_Started_BlockDodge(const FVector2D& InputVector)
{
	if (!stateMachineComp) return;
	if (InputVector.IsNearlyZero()) stateMachineComp->RequestBlockStart();
	else stateMachineComp->RequestDodge(InputVector);
}

void APlayer_Base::Input_Released_BlockDodge()
{
	if (stateMachineComp && stateMachineComp->HasExactActiveTag(CombatTags::Block)) stateMachineComp->RequestBlockStop();
}

void APlayer_Base::Input_Started_Jump()
{
	if (stateMachineComp) stateMachineComp->RequestJumpStart();
	else if (locoComp) locoComp->JumpStart();
	else Jump();
}

void APlayer_Base::Input_Released_Jump()
{
	if (stateMachineComp) stateMachineComp->RequestJumpStop();
	else if (locoComp) locoComp->JumpStop();
	else StopJumping();
}

void APlayer_Base::Input_Triggered_Look_Mouse(const FVector2D& InputVector)
{
	if (stateMachineComp) stateMachineComp->RequestLookMouse(InputVector);
	else if (playerCamComp) playerCamComp->AddLookMouseInput(InputVector);
	else if (UWorld* world = GetWorld())
	{
		const float DT = world->GetDeltaSeconds();

		AddControllerYawInput(InputVector.X * 45.0f * DT);
		AddControllerPitchInput(InputVector.Y * 45.0f * DT);
	}
}

void APlayer_Base::Input_Triggered_Look_Stick(const FVector2D& InputVector)
{
	if (stateMachineComp) stateMachineComp->RequestLookStick(InputVector);
	else if (playerCamComp) playerCamComp->AddLookStickInput(InputVector);
	else if (UWorld* world = GetWorld())
	{
		const float DT = world->GetDeltaSeconds();

		AddControllerYawInput(InputVector.X * 45.0f * DT);
		AddControllerPitchInput(InputVector.Y * 45.0f * DT);
	}
}

void APlayer_Base::Input_Started_Move(const FVector2D& InputVector)
{
	if (stateMachineComp)
	{
		if (stateMachineComp->HasExactActiveTag(CombatTags::Block) && !InputVector.IsNearlyZero()) stateMachineComp->RequestDodge(InputVector);
		else stateMachineComp->RequestMove(InputVector);
	}
	else if (locoComp) locoComp->AddMoveInput(InputVector);
	else
	{
		FRotator ControlRot = GetControlRotation();
		ControlRot.Pitch = 0.f;
		ControlRot.Roll  = 0.f;

		const FVector Right   = UKismetMathLibrary::GetRightVector(ControlRot);
		const FVector Forward = UKismetMathLibrary::GetForwardVector(ControlRot);

		AddMovementInput(Right,   InputVector.X);
		AddMovementInput(Forward, InputVector.Y);
	}
}

void APlayer_Base::Input_Triggered_Move(const FVector2D& InputVector)
{
	if (stateMachineComp) stateMachineComp->RequestMove(InputVector);
	else if (locoComp) locoComp->AddMoveInput(InputVector);
	else
	{
		FRotator ControlRot = GetControlRotation();
		ControlRot.Pitch = 0.f;
		ControlRot.Roll  = 0.f;

		const FVector Right   = UKismetMathLibrary::GetRightVector(ControlRot);
		const FVector Forward = UKismetMathLibrary::GetForwardVector(ControlRot);

		AddMovementInput(Right,   InputVector.X);
		AddMovementInput(Forward, InputVector.Y);
	}
}

void APlayer_Base::Input_Started_ToggleLockOn()
{
	if (stateMachineComp) stateMachineComp->RequestToggleLockOn();
	else if (playerTargettingComp) playerTargettingComp->ToggleLockOn();
}

void APlayer_Base::HandleActorDeath(AActor* Actor)
{
	if (playerTargettingComp && playerTargettingComp->GetCurrentTarget() == Actor) playerTargettingComp->LockOff();
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

/************************************ Combat Interface Functions *************************************/
int APlayer_Base::GetPowerLevel() const {return combatResComp ? combatResComp->powerLvl : 0;}
int APlayer_Base::GetPowerLevelMax() const {return combatResComp ? combatResComp->powerLvlMax : 3;}
AActor* APlayer_Base::GetCurrentTarget() const { return playerTargettingComp ? playerTargettingComp->GetCurrentTarget() : nullptr; }



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