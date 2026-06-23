// Fill out your copyright notice in the Description page of Project Settings.
#include "Player_Base.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/SpringArmComponent.h"

#include "../Tags/CharacterStateTags.h"
#include "../Combat/Shared/CombatResolutionComponent.h"
#include "../Combat/Shared/CombatTraceComponent.h"
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

	combatComp = CreateDefaultSubobject<UPlayerCombatComponent>(TEXT("Combat"));
	combatResComp = CreateDefaultSubobject<UCombatResolutionComponent>(TEXT("Combat Resolution"));
	combatTraceComp = CreateDefaultSubobject<UCombatTraceComponent>(TEXT("Combat Trace"));
	locoComp = CreateDefaultSubobject<ULocomotionComponent>(TEXT("Locomotion"));
	playerCamComp = CreateDefaultSubobject<UPlayerCamComponent>(TEXT("Player Camera"));
	playerCombatCancelComp = CreateDefaultSubobject<UPlayerCombatCancelComponent>(TEXT("Player Combat Cancel"));
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

void APlayer_Base::PlayerInput(EPlayerInput PlayerInput, const FVector2D& InputVector)
{
	if (!stateMachineComp) return;

	const FGameplayTag CharacterActionTag = stateMachineComp->ResolvePlayerInput(PlayerInput, InputVector);

	if (CharacterActionTag.MatchesTag(CharacterActionTags::Attack) && combatComp) combatComp->Attack(CharacterActionTag, InputVector);
	else if (CharacterActionTag.MatchesTagExact(CharacterActionTags::BlockStart) && combatComp) combatComp->BlockStart();
	else if (CharacterActionTag.MatchesTagExact(CharacterActionTags::BlockFinish) && combatComp) combatComp->BlockStop();
	else if (CharacterActionTag.MatchesTagExact(CharacterActionTags::Dodge) && combatComp) combatComp->Dodge(InputVector);
	else if (CharacterActionTag.MatchesTagExact(CharacterActionTags::JumpStart) && locoComp) locoComp->JumpStart();
	else if (CharacterActionTag.MatchesTagExact(CharacterActionTags::JumpFinish) && locoComp) locoComp->JumpStop();
	else if (CharacterActionTag.MatchesTagExact(CharacterActionTags::LookMouse) && playerCamComp) playerCamComp->AddLookMouseInput(InputVector);
	else if (CharacterActionTag.MatchesTagExact(CharacterActionTags::LookStick) && playerCamComp) playerCamComp->AddLookStickInput(InputVector);
	else if (CharacterActionTag.MatchesTagExact(CharacterActionTags::Move) && locoComp) locoComp->Move(InputVector);
}

/*void APlayer_Base::Input_AttackHeavy_Started(const FVector2D& InputVector)
{
	if (stateMachineComp) stateMachineComp->ResolvePlayerInput(EPlayerInput::AttackHeavyStart, InputVector);
}

void APlayer_Base::Input_AttackHeavy_OnGoing(const FVector2D& InputVector)
{
	if (stateMachineComp) stateMachineComp->ResolvePlayerInput(EPlayerInput::AttackHeavyOngoing, InputVector);
}

void APlayer_Base::Input_AttackHeavy_Completed(const FVector2D& InputVector)
{
	if (stateMachineComp) stateMachineComp->ResolvePlayerInput(EPlayerInput::AttackHeavyComplete, InputVector);
}

void APlayer_Base::Input_AttackLight_Started(const FVector2D& InputVector)
{
	if (stateMachineComp) stateMachineComp->ResolvePlayerInput(EPlayerInput::AttackLightStart, InputVector);
}

void APlayer_Base::Input_AttackLight_OnGoing(const FVector2D& InputVector)
{
	if (stateMachineComp) stateMachineComp->ResolvePlayerInput(EPlayerInput::AttackLightOngoing, InputVector);
}

void APlayer_Base::Input_AttackLight_Completed(const FVector2D& InputVector)
{
	if (stateMachineComp) stateMachineComp->ResolvePlayerInput(EPlayerInput::AttackLightComplete, InputVector);
}

void APlayer_Base::Input_BlockDodge_Started(const FVector2D& InputVector)
{
	if (stateMachineComp)
	{
		if (InputVector.IsNearlyZero()) stateMachineComp->ResolvePlayerInput(EPlayerInput::BlockStart, InputVector);
		else stateMachineComp->ResolvePlayerInput(EPlayerInput::DodgeStart);
	}
}

void APlayer_Base::Input_BlockDodge_OnGoing(const FVector2D& InputVector)
{
	if (stateMachineComp && InputVector.IsNearlyZero()) stateMachineComp->ResolvePlayerInput(EPlayerInput::BlockTrigger, InputVector);
}

void APlayer_Base::Input_Jump_Started(const FVector2D &InputVector)
{
	if (stateMachineComp) stateMachineComp->ResolvePlayerInput(EPlayerInput::JumpStart, InputVector);
	else if (locoComp) locoComp->JumpStart();
	else Jump();
}

void APlayer_Base::Input_Jump_Completed(const FVector2D& InputVector)
{
	if (stateMachineComp) stateMachineComp->ResolvePlayerInput(EPlayerInput::JumpComplete, InputVector);
	else if (locoComp) locoComp->JumpStop();
	else StopJumping();
}

void APlayer_Base::Input_LockOnOff_Started(const FVector2D& InputVector)
{
	if (stateMachineComp) stateMachineComp->ResolvePlayerInput(EPlayerInput::LockOnOffStart, InputVector);
}

void APlayer_Base::Input_LookMouse_Triggered(const FVector2D& InputVector)
{
	if (stateMachineComp) stateMachineComp->ResolvePlayerInput(EPlayerInput::LookMouseTrigger, InputVector);
	else if (playerCamComp) playerCamComp->AddLookMouseInput(InputVector);
	else if (UWorld* world = GetWorld())
	{
		const float DT = world->GetDeltaSeconds();

		AddControllerYawInput(InputVector.X * 45.0f * DT);
		AddControllerPitchInput(InputVector.Y * 45.0f * DT);
	}
}

void APlayer_Base::Input_LookStick_Triggered(const FVector2D& InputVector)
{
	if (stateMachineComp) stateMachineComp->ResolvePlayerInput(EPlayerInput::LookStickTrigger, InputVector);
	else if (playerCamComp) playerCamComp->AddLookStickInput(InputVector);
	else if (UWorld* world = GetWorld())
	{
		const float DT = world->GetDeltaSeconds();

		AddControllerYawInput(InputVector.X * 45.0f * DT);
		AddControllerPitchInput(InputVector.Y * 45.0f * DT);
	}
}

void APlayer_Base::Input_Move_Triggered(const FVector2D& InputVector)
{
	if (stateMachineComp) stateMachineComp->ResolvePlayerInput(EPlayerInput::MoveTrigger, InputVector);
	else if (locoComp) locoComp->Move(InputVector);
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
}*/

void APlayer_Base::HandleActorDeath(AActor* Actor)
{
	if (playerTargettingComp && playerTargettingComp->GetCurrentTarget() == Actor) playerTargettingComp->LockOff();
}

/************************************ Combat Interface Functions *************************************/
AActor* APlayer_Base::GetCurrentTarget() const { return playerTargettingComp ? playerTargettingComp->GetCurrentTarget() : nullptr; }
bool APlayer_Base::GetLockedOn() const { return playerTargettingComp ? playerTargettingComp->GetLockedOn() : false; }

/************************************ Damageable Interface Functions ********************************/
bool APlayer_Base::IsAlive() const { return statsComp ? statsComp->IsAlive() : false; }

void APlayer_Base::ReceiveHit(FAtkHitData& HitData)
{
	if (!IsAlive()) return;

	if (bDebug)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, FString::Printf(TEXT("[%s] Received Hit from %s"), *GetName(), *HitData.attacker->GetName()));
		UE_LOG(LogTemp, Warning, TEXT("ReceiveHit CALLED on %s by %s"), *GetName(), *GetNameSafe(HitData.attacker));
	}
	
	const bool bHasCombatRes = combatResComp != nullptr;
	const bool bHasCombatComp = combatComp != nullptr;
	const bool bHasStateMachine = stateMachineComp != nullptr;
	const bool bHasStats = statsComp != nullptr;
	HitData.resolvedReaction = StateActionTags::None;

	// --- Resolve Blocking ---
	if (bHasCombatComp) combatComp->ReceieveHit(HitData);

	// --- Resolve Poise ---
	if (bHasCombatRes) combatResComp->RecieveHit(HitData);

	// --- Apply Damage ---
	if (bHasStats) HitData.dmgHPDealt = statsComp->ApplyDamage(HitData.dmgHP, HitData.penetration);

	if (bDebug)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, FString::Printf(TEXT("[%s] HitData.resoleved reaction = %s"), *GetName(), *HitData.resolvedReaction.ToString()));
		UE_LOG(LogTemp, Display, TEXT("[%s] HitData.resolvedReaction = %s"), *GetName(), *HitData.resolvedReaction.ToString());
	}

	// --- State Machine ---
	const bool bHasReaction = HitData.resolvedReaction != StateActionTags::None;
	if (bHasReaction && bHasStateMachine) stateMachineComp->HandleReceiveHit(HitData);

	OnHit.Broadcast(HitData);
}