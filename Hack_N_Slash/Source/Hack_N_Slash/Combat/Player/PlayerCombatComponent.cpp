#include "PlayerCombatComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

#include "../../Animation/AnimInstances/BaseCharAnimInstance.h"
#include "../../Interfaces/CombatInstigator.h"
#include "../Shared/CombatResolutionComponent.h"
#include "../../Combat/Shared/CombatTraceComponent.h"
#include "../../Interfaces/Damageable.h"
#include "../Enemy/EnemyCombatComponent.h"
#include "../../Structs/FAtkHitData.h"
#include "../../Characters/Player/PlayerInputComponent.h"
#include "../../Characters/Shared/LocomotionComponent.h"
#include "../../Combat/Player/PlayerTargettingComponent.h"
#include "../../Characters/Shared/StateMachineComponent.h"

UPlayerCombatComponent::UPlayerCombatComponent() { PrimaryComponentTick.bCanEverTick = false; }

void UPlayerCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	EnsureReferences();

	if (ownerChar) ownerChar->LandedDelegate.AddDynamic(this, &UPlayerCombatComponent::HandleLanded);
	currentAtkData = nullptr;
	move = FVector2D::ZeroVector;
}

void UPlayerCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) { Super::TickComponent(DeltaTime, TickType, ThisTickFunction); }

void UPlayerCombatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* world = GetWorld()) world->GetTimerManager().ClearAllTimersForObject(this);
	if (ownerChar) ownerChar->LandedDelegate.RemoveDynamic(this, &UPlayerCombatComponent::HandleLanded);
	Super::EndPlay(EndPlayReason);
}

bool UPlayerCombatComponent::EnsureReferences()
{
    if (!ownerChar) ownerChar = Cast<ACharacter>(GetOwner());
    if (!ownerChar)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UPlayerCombatComponent] Owner is not an ACharacter: %s"), *GetNameSafe(GetOwner()));
        return false;
    }

	if (!animInst)
	{
		if (USkeletalMeshComponent* skeletalMeshComp = ownerChar->GetMesh()) animInst = Cast<UBaseCharAnimInstance>(skeletalMeshComp->GetAnimInstance());
	}
	if (!animInst)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UPlayerCombatComponent] Owner's skeletal mesh does not have a valid base char animation instance: %s"), *GetNameSafe(ownerChar));
		return false;
	}

    if (!moveComp) moveComp = ownerChar->GetCharacterMovement();
    if (!moveComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UPlayerCombatComponent] No CharacterMovementComponent on: %s"), *GetNameSafe(ownerChar));
        return false;
    }

	if (!inputComp) inputComp = ownerChar->FindComponentByClass<UPlayerInputComponent>();
	if (!inputComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UPlayerCombatComponent] No PlayerInputComponent on: %s"), *GetNameSafe(ownerChar));
        return false;
    }

	if (!stateMachineComp) stateMachineComp = ownerChar ? ownerChar->FindComponentByClass<UStateMachineComponent>() : nullptr;
	if (!stateMachineComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UPlayerCombatComponent] No StateMachineComponent on: %s"), *GetNameSafe(ownerChar));
        return false;
    }

	if (!iCmbtInst) iCmbtInst = Cast<ICombatInstigator>(ownerChar);
	if (!iCmbtInst)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UPlayerCombatComponent] Owner does not implement ICombatInstigator: %s"), *GetNameSafe(ownerChar));
		return false;
	}

	if (!locoComp) locoComp = ownerChar->FindComponentByClass<ULocomotionComponent>();
	if (!combatResComp) combatResComp = ownerChar->FindComponentByClass<UCombatResolutionComponent>();
	if (!playerTargettingComp) playerTargettingComp = ownerChar->FindComponentByClass<UPlayerTargettingComponent>();
	if (!traceComp) traceComp = ownerChar->FindComponentByClass<UCombatTraceComponent>();

    return true;
}

bool UPlayerCombatComponent::IsAtkContextValid(const FPlayerAtkData& AtkData, const FGameplayTag& CharacterAction, const FVector2D& Move) const
{
	if (!AtkData.bUnlocked) return false;
	
	bool bActionMatch = AtkData.actionTag == CharacterAction;

	bool bLockRequirementMatch = false;
	switch (AtkData.lockRequirement)
	{
	case ELockRequirement::Either:
		bLockRequirementMatch = true;
		break;

	case ELockRequirement::Off:
		bLockRequirementMatch = playerTargettingComp && !playerTargettingComp->GetLockedOn();
		break;
	
	case ELockRequirement::On:
		bLockRequirementMatch = playerTargettingComp && playerTargettingComp->GetLockedOn();
		break;
	
	default:
		break;
	}

	bool bLStickMovementMatch = false;

	if (AtkData.lStickMotion == EStickMotion::None) bLStickMovementMatch = inputComp->PerformedDirection(AtkData.lStickDirection, Move);
	else bLStickMovementMatch = inputComp->PerformedMotion(AtkData.lStickMotion);

	bool bMovementStateMatch = AtkData.movementState.IsValid() && iCmbtInst->HasTag(AtkData.movementState);
	
    return bActionMatch && bLockRequirementMatch && bLStickMovementMatch && bMovementStateMatch;
}

void UPlayerCombatComponent::Attack(const FGameplayTag& ActionTag, const FVector2D& Move, bool bBuffer)
{
	if (!EnsureReferences() || !activeAtkDT) return;

	// 1: Get Potential atk Data
	FPlayerAtkData* nextAtkData = GetPotentialAtkData(ActionTag, Move);
	if (!nextAtkData || !nextAtkData->montage) return;


	// 2: Try to enter attack state
	UActionState* attackState = stateMachineComp->GetActionStateByTag(Tags::StateMachine::Action::Combat::Attack);
	if (!stateMachineComp->ChangeActionState(attackState, false))
	{
		if (!bBuffer) inputComp->SetActionBuffer(nextAtkData->actionTag, Move); // Only set a new buffer if this function isn't being called by a buffer
		return;
	}
	else inputComp->ClearActionBuffer(); // Performing this action, so clear any buffered aciton if it exists
	

	// 3: Perform the attack
	PerformAttack(nextAtkData, Move);
}

FPlayerAtkData* UPlayerCombatComponent::GetPotentialAtkData(const FGameplayTag& ActionTag, const FVector2D& Move)
{
	if (!EnsureReferences() || !activeAtkDT) return nullptr;

	FPlayerAtkData* nextAtkData = nullptr;
	if (!currentAtkData)
	{
		static const FString contextStr(TEXT("[PlayerCombatComp] Getting Initial Attack"));
		TArray<FName> rowNames = activeAtkDT->GetRowNames();
		for (FName row : rowNames)
		{
			FPlayerAtkData* rowData = activeAtkDT->FindRow<FPlayerAtkData>(row, contextStr);
			if (!rowData) continue;

			if (IsAtkContextValid(*rowData, ActionTag, Move) && (!nextAtkData || rowData->priority > nextAtkData->priority)) nextAtkData = rowData;
		}
	}
	else
	{
		static const FString contextStr(TEXT("[PlayerCombatComp] Getting Next Attack"));
		for (FName atkCandidate : currentAtkData->nextAtkIDs)
		{
			FPlayerAtkData* candidateData = activeAtkDT->FindRow<FPlayerAtkData>(atkCandidate, contextStr);
			if (!candidateData) continue;

			if (IsAtkContextValid(*candidateData, ActionTag, Move) && (!nextAtkData || candidateData->priority > nextAtkData->priority)) nextAtkData = candidateData;
		}
	}

	if (!nextAtkData && bDebug)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("[UPlayerCombatComponent] No valid attack found"));
		UE_LOG(LogTemp, Warning, TEXT("[UPlayerCombatComponent] No valid attack found"));
	}
	return nextAtkData;
}

void UPlayerCombatComponent::PerformAttack(FPlayerAtkData* AtkData, const FVector2D& Move)
{
	currentAtkData = AtkData; // Set current attack data to new attack data
	move = Move; // Set current move stick value to new move stick value. Used by anim notify for player targetting

	// Play the attack montage and set the end delegate
	FOnMontageEnded MontageEndedDelegate;
	MontageEndedDelegate.BindUObject(this, &UPlayerCombatComponent::OnAttackMontageEnded);
	if (!animInst->PlayMontageHNS(AtkData->montage, AtkData->montageSection))
	{
		ClearAtkData();
		stateMachineComp->ClearActionState();
		if (locoComp) locoComp->ClearWarpData();
		if (playerTargettingComp) playerTargettingComp->ClearCurrentTarget();
		if (traceComp) traceComp->ClearHitActors();
		return;
	}
	animInst->Montage_SetEndDelegate(MontageEndedDelegate, AtkData->montage);
}

void UPlayerCombatComponent::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!EnsureReferences()) return;
	
	if (traceComp) traceComp->ClearHitActors();
	
	if (bInterrupted)
	{
		if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("[PlayerCombatComp] Attack Montage: Interrupted"));
		if (iCmbtInst->HasTag(Tags::StateMachine::Action::Combat::Attack)) return; // If still in attack state, don't clear motion warp data because new warp data is often applied at this point
	}
	else if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("[PlayerCombatComp] Attack Montage: Finished"));

	ClearAtkData();
	if (locoComp) locoComp->ClearWarpData();
	if (playerTargettingComp) playerTargettingComp->ClearCurrentTarget();
}

void UPlayerCombatComponent::ClearAtkData()
{
	currentAtkData = nullptr;
	move = FVector2D::ZeroVector;
}

bool UPlayerCombatComponent::CanPerfectBlock() const { return bPerfectBlockUnlocked && blockActionInput == Tags::PlayerAction::BlockStart; }

void UPlayerCombatComponent::BlockStart(bool bBuffer)
{
	if (!EnsureReferences() || !activeBlockMontage) return;

	blockActionInput = Tags::PlayerAction::BlockStart;
	if (stateMachineComp->ChangeActionState(stateMachineComp->GetActionStateByTag(Tags::StateMachine::Action::Combat::Block), false)) inputComp->ClearActionBuffer();
	else if (!bBuffer) inputComp->SetActionBuffer(Tags::PlayerAction::BlockStart);
}

void UPlayerCombatComponent::BlockHold(bool bBuffer)
{
	if (!EnsureReferences() || !activeBlockMontage) return;

	blockActionInput = Tags::PlayerAction::BlockHold;
	if (stateMachineComp->ChangeActionState(stateMachineComp->GetActionStateByTag(Tags::StateMachine::Action::Combat::Block), false)) inputComp->ClearActionBuffer();
	else if (!bBuffer) inputComp->SetActionBuffer(Tags::PlayerAction::BlockHold);
}

void UPlayerCombatComponent::BlockStop()
{
	if (!EnsureReferences()) return;

	blockActionInput = Tags::PlayerAction::BlockRelease;
	animInst->PlayMontageHNS(activeBlockMontage, TEXT("End"));
	stateMachineComp->ClearActionState();
}

void UPlayerCombatComponent::StartRegenBlockCount()
{
	UWorld* world = GetWorld();
	if (!world) return;

	bBlockBroken = false;
	world->GetTimerManager().SetTimer(TH_BlockRegen, this, &UPlayerCombatComponent::RegenBlockCount, blockRegenRate, true);
}

void UPlayerCombatComponent::RegenBlockCount()
{
	if (iCmbtInst && iCmbtInst->HasTag(Tags::StateMachine::Action::Combat::Block)) return;
	
	--blockCount;
	blockCount = FMath::Clamp(blockCount, 0, maxBlockHits);

	if (blockCount <= 0) if (UWorld* world = GetWorld()) world->GetTimerManager().ClearTimer(TH_BlockRegen);
}

void UPlayerCombatComponent::Dodge(const FVector2D& Move, bool bBuffer)
{
	if (!EnsureReferences() || !locoComp) return;

	UWorld* world = GetWorld();
	if (!world) return;

	// Try to enter the dodge state
	UActionState* dodgeState = stateMachineComp->GetActionStateByTag(Tags::StateMachine::Action::Combat::Dodge);
	if (!stateMachineComp->ChangeActionState(dodgeState, false))
	{
		if (!bBuffer) inputComp->SetActionBuffer(Tags::PlayerAction::Dodge, Move); // Only set a new buffer if this function isn't being called by a buffer
		return;
	}
	else inputComp->ClearActionBuffer(); // Performing this action, so clear any buffered aciton if it exists

	UAnimMontage* dodgeMont = nullptr;
	FVector dodgeForce = FVector::ZeroVector;

	AActor* target = playerTargettingComp ? playerTargettingComp->GetCurrentTarget() : nullptr;

	// Input direction relative to camera / target
	FVector localForward, localRight;
	const FVector dodgeWorldDir = inputComp->GetInputWorldDirRelativeToCamOrTarget(Move, localForward, localRight, target);

	dodgeForce = dodgeWorldDir * (distance / duration); // Calculate the necessary force to cover the dodge distance in the desired duration

	if (iCmbtInst->IsAirborne())
	{
		++airDodgeCount;
		airDodgeCount = FMath::Clamp(airDodgeCount, 0, maxAirDodges);
		dodgeMont = airDodgeMont;
	}
	else dodgeMont = groundDodgeMont;

	ownerChar->SetActorRotation(dodgeWorldDir.Rotation()); // Rotate in the direction of the dodge
	dodgeForce = ownerChar->GetActorForwardVector() * (distance / duration); // Calculate the necessary force to cover the dodge distance in the desired duration

	if (!animInst->PlayMontageHNS(dodgeMont))
	{
		stateMachineComp->ClearActionState();
		return;
	}

	currentDodgeMont = dodgeMont;

	UAsyncRootMovement* aSyncRootMovement = locoComp->ApplyRootMotionSourceConstant(duration, dodgeForce, setVelocityOnFinish, clampVelocityOnFinish, velocityOnFinishMode, strengthOverTime, bIsAdditive);
	if (!aSyncRootMovement)
	{
		stateMachineComp->ClearActionState();
		animInst->Montage_Stop(0.25f, currentDodgeMont);
		currentDodgeMont = nullptr;
		return;
	}
	aSyncRootMovement->OnComplete.AddDynamic(this, &UPlayerCombatComponent::EndDodge);
}

void UPlayerCombatComponent::EndDodge(UAsyncRootMovement* RootMovement)
{
	if (!EnsureReferences()) return;
	
	if (currentDodgeMont) animInst->Montage_Resume(currentDodgeMont);
	currentDodgeMont = nullptr;
}

void UPlayerCombatComponent::HandleLanded(const FHitResult& Hit) { airDodgeCount = 0; }

void UPlayerCombatComponent::ReceieveHit(FAtkHitData& HitData)
{
	if (!EnsureReferences() || !combatResComp) return;

	UWorld* world = GetWorld();
	if (!world) return;

	bool bBlocking = iCmbtInst->HasTag(Tags::StateMachine::Action::Combat::Block, true);
	if (!bBlocking) return;
	
	bool bIsImmune = combatResComp->GetVulnerability() == ECombatVulnerability::Immune;

	UEnemyCombatComponent* enemyCmbtComp = HitData.attacker ? HitData.attacker->FindComponentByClass<UEnemyCombatComponent>() : nullptr;
	bool bAtkerHasSuperArmor = enemyCmbtComp && enemyCmbtComp->HasSuperArmor();
	
	if (bAtkerHasSuperArmor && !bCanBlockSuperArmor && !bIsImmune) HitData.resolvedReaction = Tags::StateMachine::Action::Reaction::BlockBreak; // If can't attacker has super armor and can't block it, block breaks
	else if (bPerfectBlockWindow) // Perfect Block
	{
		HitData.resolvedReaction = Tags::StateMachine::Action::Reaction::BlockPerfect;
		blockCount = 0;
		if (IDamageable* iDmgblAtkr = Cast<IDamageable>(HitData.damager)) iDmgblAtkr->Countered(ownerChar, "Perfect Block"); // Tell the damager they were countered
	}
	else if (bIsImmune) HitData.resolvedReaction = Tags::StateMachine::Action::Reaction::BlockHit; // If immune, just play block hit
	else // Try Block
	{
		++blockCount;
		if (blockCount > maxBlockHits) HitData.resolvedReaction = Tags::StateMachine::Action::Reaction::BlockBreak;
		else HitData.resolvedReaction = Tags::StateMachine::Action::Reaction::BlockHit;
	}

	if (HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::BlockBreak)
	{
		HitData.dmg = 0.0f; // Block broken means take half damage
		bBlockBroken = true;
		blockCount = maxBlockHits;
	}
	else if (HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::BlockHit) HitData.dmg = 0.0f; // Blocked the hit, so take no damage

	FTimerManager& timerManager = world->GetTimerManager();
	timerManager.ClearTimer(TH_BlockRegenDelay);
	timerManager.ClearTimer(TH_BlockRegen);
	timerManager.SetTimer(TH_BlockRegenDelay, this, &UPlayerCombatComponent::StartRegenBlockCount, blockRegenDelay, false);
}