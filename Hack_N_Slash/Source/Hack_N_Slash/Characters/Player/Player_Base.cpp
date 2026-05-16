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
	playerCombatComp = CreateDefaultSubobject<UPlayerCombatComponent>(TEXT("Combat"));
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

	UCharacterMovementComponent* moveComp = GetCharacterMovement();
	if (moveComp)
	{
		moveComp->bOrientRotationToMovement = true;
		moveComp->bUseControllerDesiredRotation = false;
	}

	UCameraComponent* camComp = FindComponentByClass<UCameraComponent>();
	if (camComp) camComp->bUsePawnControlRotation = false;
	
	USpringArmComponent* springArmComp = FindComponentByClass<USpringArmComponent>();
	if (!springArmComp) return;
	springArmComp->bDoCollisionTest = true;
	springArmComp->bUsePawnControlRotation = true;
}

void APlayer_Base::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlayer_Base::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* world = GetWorld()) world->GetTimerManager().ClearAllTimersForObject(this);
	Super::EndPlay(EndPlayReason);
}

// Called to bind functionality to input
void APlayer_Base::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void APlayer_Base::Input_Started_AttackHeavy(const FVector2D& InputVector)
{
	if (stateMachineComp) stateMachineComp->RequestAttackPlayer(InputVector, EPlayerAction::AttackHeavyStart);
	else if (playerCombatComp) playerCombatComp->AttackHeavyStart(InputVector);
}

void APlayer_Base::Input_Started_AttackLight(const FVector2D& InputVector)
{
	if (stateMachineComp) stateMachineComp->RequestAttackPlayer(InputVector, EPlayerAction::AttackLightStart);
	else if (playerCombatComp) playerCombatComp->AttackLightStart(InputVector);
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
	UWorld* world = GetWorld();
	if (!world) return;

	if (stateMachineComp)
	{
		if (stateMachineComp->HasExactActiveTag(CombatTags::Block) && !InputVector.IsNearlyZero()) stateMachineComp->RequestDodge(InputVector);
		else
		{
			world->GetTimerManager().SetTimer(TH_Input_Move, FTimerDelegate::CreateLambda([this, InputVector] { stateMachineComp->RequestMove(InputVector); }), inputRegisterTime, false);
			//stateMachineComp->RequestMove(InputVector);
		}
	}
	else if (locoComp)
	{
		world->GetTimerManager().SetTimer(TH_Input_Move, FTimerDelegate::CreateLambda([this, InputVector] { locoComp->AddMoveInput(InputVector); }), inputRegisterTime, false);
		//locoComp->AddMoveInput(InputVector);
	}
	else
	{
		world->GetTimerManager().SetTimer(TH_Input_Move, FTimerDelegate::CreateLambda([this, InputVector]
			{
				FRotator ControlRot = GetControlRotation();
				ControlRot.Pitch = 0.f;
				ControlRot.Roll  = 0.f;

				const FVector Right   = UKismetMathLibrary::GetRightVector(ControlRot);
				const FVector Forward = UKismetMathLibrary::GetForwardVector(ControlRot);

				AddMovementInput(Right,   InputVector.X);
				AddMovementInput(Forward, InputVector.Y);
			}), inputRegisterTime, false);
	}
}

void APlayer_Base::Input_Triggered_Move(const FVector2D& InputVector)
{
	UWorld* world = GetWorld();
	if (!world || world->GetTimerManager().IsTimerActive(TH_Input_Move)) return;

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

/************************************ Combat Interface Functions *************************************/
AActor* APlayer_Base::GetCurrentTarget() const { return playerTargettingComp ? playerTargettingComp->GetCurrentTarget() : nullptr; }
int APlayer_Base::GetPoise() const {return combatResComp ? combatResComp->poise : 0;}



/************************************ Damageable Interface Functions ********************************/
bool APlayer_Base::IsAlive() const { return statsComp ? statsComp->IsAlive() : false; }

void APlayer_Base::ReceiveHit(FAtkHitData& HitData)
{
	if (!IsAlive()) return;
	
	const bool bHasCombatRes = combatResComp != nullptr;
	const bool bHasCombatComp = playerCombatComp != nullptr;
	const bool bHasStateMachine = stateMachineComp != nullptr;
	const bool bHasStats = statsComp != nullptr;

	// --- Resolve Super Armor and Power Level ---
	if (bHasCombatRes) combatResComp->RecieveHit(HitData);

	// --- Resolve Blocking ---
	if (bHasCombatComp) playerCombatComp->ReceieveHit(HitData);

	// --- Apply Damage ---
	if (bHasStats)
	{
		HitData.dmgHPDealt = statsComp->ApplyDamage(HitData.dmgHP, HitData.penetration);
		if (!IsAlive()) HitData.resolvedReaction = HitTags::Dead;
	}

	if (bDebug)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, FString::Printf(TEXT("[%s] HitData.resoleved reaction = %s"), *GetName(), *HitData.resolvedReaction.ToString()));
		UE_LOG(LogTemp, Display, TEXT("[%s] HitData.resolvedReaction = %s"), *GetName(), *HitData.resolvedReaction.ToString());
	}

	// --- State Machine ---
	const bool bHasReaction = HitData.resolvedReaction != ActionTags::None;
	if (bHasReaction && bHasStateMachine) stateMachineComp->OnReceiveHit(HitData);
}