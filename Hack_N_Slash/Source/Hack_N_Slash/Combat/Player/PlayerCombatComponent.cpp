#include "PlayerCombatComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

#include "../../Animation/AnimInstances/BaseCharAnimInstance.h"
#include "../../Interfaces/CombatInstigator.h"
#include "../Shared/CombatResolutionComponent.h"
#include "../../Combat/Shared/CombatTraceComponent.h"
#include "../../Interfaces/Damageable.h"
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
	SwitchChakraNature(EChakraNature::None);
}

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

bool UPlayerCombatComponent::IsAtkContextValid(const FPlayerAtkData& AtkData, const FGameplayTag& PlayerAction, const FVector2D& Move) const
{	
	const bool bActionMatch = AtkData.actionTag == PlayerAction; // Does the player action match this attack's required action? EX: Attack.Heavy.Hold

	const bool bInputDelayMatch = !AtkData.bInputDelay || bAtkDelayWindow; // Attacks requiring the input-delay window are only valid while the window is active

	bool bInputHoldTimeMatch = false; // Was the input held long enough if it requires a hold?
	if (AtkData.actionTag.MatchesTag(Tags::PlayerAction::AttackHeavy)) bInputHoldTimeMatch = inputComp->GetHeldTimeAtkHeavy() >= AtkData.holdTime;
	else if (AtkData.actionTag.MatchesTag(Tags::PlayerAction::AttackLight)) bInputHoldTimeMatch = inputComp->GetHeldTimeAtkLight() >= AtkData.holdTime;
	else bInputHoldTimeMatch = true; // Currently no other inputs require this

	bool bLockRequirementMatch = false; // Does this attack require the player to be locked on/off?
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

	// Does the player's movement motion match this attacks's required movement motion?
	const bool bMoveInputMatch = inputComp->PerformedMotion(AtkData.moveInputMotion, Move);

	const bool bMovementStateMatch = iCmbtInst->HasTag(AtkData.movementState); // Is the player in the required movement state for this attacks. EX: Airborne
	
    return bActionMatch && bInputDelayMatch && bInputHoldTimeMatch && bLockRequirementMatch && bMoveInputMatch && bMovementStateMatch; // Needs everything to be true
}

bool UPlayerCombatComponent::HasHigherAtkPriority(FPlayerAtkData* CurrentChoice, FPlayerAtkData* EvaluatingChoice) const
{
	if (!EvaluatingChoice) return false; // Null so obviously we can't choose it

	if (!CurrentChoice) return true; // Current choice is null, so obviously choose the new one

	if (EvaluatingChoice->moveInputPriority > CurrentChoice->moveInputPriority) return true; // Move prioirty is the most important
	
	return EvaluatingChoice->bInputDelay && !CurrentChoice->bInputDelay; // At this point check input timing priority
}

void UPlayerCombatComponent::Attack(const FGameplayTag& ActionTag, const FVector2D& Move, bool bBuffer)
{
	if (!EnsureReferences() || !activeAtkDT) return;

	// 1: Get Potential atk Data
	FPlayerAtkData* potentialAtkData = GetPotentialAtkData(ActionTag, Move);
	if (!potentialAtkData || !potentialAtkData->montage) return;


	// 2: Try to enter attack state
	UActionState* attackState = stateMachineComp->GetActionStateByTag(Tags::StateMachine::Action::Combat::Attack);
	if (!stateMachineComp->ChangeActionState(attackState, false))
	{
		// Buffered actions can't set a new buffered action. Also, don't buffer hold attacks
		if (!bBuffer && ActionTag != Tags::PlayerAction::AttackHeavyHold && ActionTag != Tags::PlayerAction::AttackLightHold) inputComp->SetActionBuffer(potentialAtkData->actionTag, Move);
		return;
	}
	else inputComp->ClearActionBuffer();


	// 3: Perform the attack
	PerformAttack(potentialAtkData, Move);

	return;
}

FPlayerAtkData* UPlayerCombatComponent::GetPotentialAtkData(const FGameplayTag& ActionTag, const FVector2D& Move)
{
	if (!EnsureReferences() || !activeAtkDT) return nullptr;

	FPlayerAtkData* nextAtkData = nullptr; // Respresents the attack we'll be selecting

	if (!currentAtkData || currentAtkData->bResetCombo) // Search every row in the active data table if the system doesn't have a current attack already OR the current attack resets the combo string
	{
		static const FString contextStr(TEXT("[PlayerCombatComp] Getting Initial Attack"));

		TArray<FName> attackNames = activeAtkDT->GetRowNames(); // Get all the attack names in the active data table
		for (FName attackName : attackNames) // Loop through each attack name
		{
			FPlayerAtkData* atkData = activeAtkDT->FindRow<FPlayerAtkData>(attackName, contextStr); // Try and find the corresponding FPlayerAtkData in the data table
			if (!atkData) continue;

			if (IsAtkContextValid(*atkData, ActionTag, Move) && HasHigherAtkPriority(nextAtkData, atkData)) nextAtkData = atkData;
		}
	}
	else // Else search through all the attacks that the current attack says you can
	{
		static const FString contextStr(TEXT("[PlayerCombatComp] Getting Next Attack"));

		for (FName atkName : currentAtkData->nextAtkIDs) // Get all the attack names that can branch form the current attack
		{
			FPlayerAtkData* atkData = activeAtkDT->FindRow<FPlayerAtkData>(atkName, contextStr);
			if (!atkData) continue;

			if (IsAtkContextValid(*atkData, ActionTag, Move) && HasHigherAtkPriority(nextAtkData, atkData)) nextAtkData = atkData;
		}
	}

	return nextAtkData;
}

void UPlayerCombatComponent::PerformAttack(FPlayerAtkData* AtkData, const FVector2D& Move)
{
	currentAtkData = AtkData; // Set current attack data to new attack data
	move = Move; // Set current move stick value to new move stick value
	bAtkDelayWindow = false; // Close the attack delay window. Need it here too because there's a slight window where this would be true when interrupted
	inputComp->ResetInputTimings(); // Performing the chosen attack, so reset input timings as they affect attack decision making

	// Play the attack montage and set the end delegate
	FOnMontageEnded MontageEndedDelegate;
	MontageEndedDelegate.BindUObject(this, &UPlayerCombatComponent::OnAttackMontageEnded);
	if (animInst->PlayMontageHNS(currentAtkData->montage, currentAtkData->montageSection))
	{
		animInst->Montage_SetEndDelegate(MontageEndedDelegate, currentAtkData->montage);
		if (UWorld* world = GetWorld())
		{
			// Even when montages add this tag at frame 0, players rapidly pressing attack may process right before the tag is added
			// So, we add it immeditely to prevent overriding the current attack before it has a chance to play
			// Quickly remove it to maintain intended behavior (i.e. don't permanently block attacking)
			iCmbtInst->AddTag(Tags::Status::ActionBlocked::Attack);
			world->GetTimerManager().SetTimer(TH_ActionBlockedAtk, [this] () { iCmbtInst->RemoveTag(Tags::Status::ActionBlocked::Attack); }, 0.1f, false);
		}
	}
	else ClearAtkData();
}

// NOTE: If interrupted by another montage, this happens slightly after the new montage starts
void UPlayerCombatComponent::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (traceComp) traceComp->ClearHitActors(); // Clear all hit actors so they can be hit again
	
	if (bInterrupted)
	{
		//if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("[PlayerCombatComp] Attack Montage: Interrupted"));

		// If interrupted by an attack, don't clear because new combat data is often applied by the new attack at this point
		if (iCmbtInst && iCmbtInst->HasTag(Tags::StateMachine::Action::Combat::Attack)) return;
	}
	//else if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("[PlayerCombatComp] Attack Montage: Finished"));

	ClearAtkData(); // Clear current attack data
	if (locoComp) locoComp->ClearWarpData();
	if (playerTargettingComp) playerTargettingComp->ClearCurrentTarget(); // Clear Soft Target. Won't do anything if locked on
}

void UPlayerCombatComponent::ClearAtkData()
{
	currentAtkData = nullptr;
	move = FVector2D::ZeroVector;
	bAtkDelayWindow = false;
}

FPlayerAtkData* UPlayerCombatComponent::GetAtkData(const FName& ID, const FString& Reason, UDataTable* DataTable) const
{
	UDataTable* atkDataTable = DataTable ? DataTable : activeAtkDT;
	return atkDataTable ? atkDataTable->FindRow<FPlayerAtkData>(ID, Reason) : nullptr;
}

FPlayerAtkData UPlayerCombatComponent::GetAtkData_BP(const FName& ID, const FString& Reason, UDataTable* DataTable) const
{
	FPlayerAtkData* atkData = GetAtkData(ID, Reason, DataTable);
	return atkData ? *atkData : FPlayerAtkData();
}

bool UPlayerCombatComponent::CanPerfectBlock() const { return blockAction == Tags::PlayerAction::BlockStart; }

void UPlayerCombatComponent::BlockStart()
{
	if (!EnsureReferences() || !activeBlockMontage) return;

	blockAction = Tags::PlayerAction::BlockStart;
	if (stateMachineComp->ChangeActionState(stateMachineComp->GetActionStateByTag(Tags::StateMachine::Action::Combat::Block), false)) inputComp->ClearActionBuffer();
}

void UPlayerCombatComponent::BlockHold()
{
	if (!EnsureReferences() || !activeBlockMontage) return;

	blockAction = Tags::PlayerAction::BlockHold;
	if (stateMachineComp->ChangeActionState(stateMachineComp->GetActionStateByTag(Tags::StateMachine::Action::Combat::Block), false)) inputComp->ClearActionBuffer();
}

void UPlayerCombatComponent::BlockStop()
{
	if (!EnsureReferences() || !activeBlockMontage) return;

	blockAction = Tags::PlayerAction::BlockRelease;
	animInst->Montage_JumpToSection("End", activeBlockMontage);
	stateMachineComp->ClearActionState();
}

void UPlayerCombatComponent::HandlePerfectBlock(FAtkHitData& HitData)
{
	FPlayerAtkData* perfectBlockData = GetAtkData(perfectBlockAtkDataID, "Perfect Block");
	if (!perfectBlockData || !perfectBlockData->montage) // If no perfect block montage, treat it as a regular block
	{
		++blockCount;
		if (blockCount > maxBlockHits) HitData.resolvedReaction = Tags::StateMachine::Action::Reaction::BlockBreak;
		else HitData.resolvedReaction = Tags::StateMachine::Action::Reaction::BlockHit;
	}
	else
	{
		HitData.resolvedReaction = Tags::StateMachine::Action::Reaction::BlockPerfect; // Set the resolved reaction to perfect block
		blockCount = 0; // Perfect blocks reset the block count
		blockAction = Tags::PlayerAction::BlockRelease; // Transition the block input to release

		// Face the damage source
		if (HitData.damager)
		{
			FRotator desiredRot = UKismetMathLibrary::FindLookAtRotation(ownerChar->GetActorLocation(), HitData.damager->GetActorLocation());
			desiredRot.Pitch = 0.0f;
			desiredRot.Roll = 0.0f;
			ownerChar->SetActorRotation(desiredRot);
		}
		else
		{
			FRotator desiredRot = UKismetMathLibrary::FindLookAtRotation(ownerChar->GetActorLocation(), HitData.hitLoc);
			desiredRot.Pitch = 0.0f;
			desiredRot.Roll = 0.0f;
			ownerChar->SetActorRotation(desiredRot);
		}

		PerformAttack(perfectBlockData, {0, 0}); // Perform the perfect block action
		if (IDamageable* iDmgblAtkr = Cast<IDamageable>(HitData.damager)) iDmgblAtkr->Countered(ownerChar, "Perfect Block"); // Tell the damager they were perfect blocked
	}
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
	else inputComp->ClearActionBuffer();

	// Get the appropriate dodge montage
	currentDodgeMont = nullptr;
	UAnimMontage* dodgeMont = nullptr;

	if (stateMachineComp->IsAirborne())
	{
		++airDodgeCount; // Increase air dodge count
		airDodgeCount = FMath::Clamp(airDodgeCount, 0, maxAirDodges);
		dodgeMont = airDodgeMont; // Use the air dodge montage
	}
	else dodgeMont = groundDodgeMont; // Use the gorund dodge montage

	// Calc dodge velocity, then rotate in that direction
	AActor* target = playerTargettingComp ? playerTargettingComp->GetCurrentTarget() : nullptr;
	FVector localForward, localRight;
	const FVector dodgeWorldDir = inputComp->GetInputWorldDirRelativeToCamOrTarget(Move, localForward, localRight, target);

	ownerChar->SetActorRotation(dodgeWorldDir.Rotation()); // Rotate in the direction of the dodge
	FVector dodgeVelocity = ownerChar->GetActorForwardVector() * (distance / duration); // Calculate the necessary velocity to cover the dodge distance in the desired duration

	// Play the dodge montage
	if (!animInst->PlayMontageHNS(dodgeMont))
	{
		stateMachineComp->ClearActionState();
		return;
	}
	currentDodgeMont = dodgeMont; // Set the current dodge montage to the calculated one

	// Apply dodge movement using a ROOT MOTION CONSTANT FORCE
	UAsyncRootMovement* aSyncRootMovement = locoComp->ApplyRootMotionSourceConstant(duration, dodgeVelocity, velocityOnFinishMode, setVelocityOnFinish, clampVelocityOnFinish, strengthOverTime, bIsAdditive);
	if (!aSyncRootMovement) // Fail-safe if the root movement failed
	{
		stateMachineComp->ClearActionState();
		animInst->Montage_Stop(0.25f, currentDodgeMont);
		currentDodgeMont = nullptr;
		return;
	}
	aSyncRootMovement->OnComplete.AddDynamic(this, &UPlayerCombatComponent::EndDodge);
	aSyncRootMovement->OnInterrupted.AddDynamic(this, &UPlayerCombatComponent::EndDodge);
}

void UPlayerCombatComponent::EndDodge(UAsyncRootMovement* RootMovement)
{
	if (currentDodgeMont && animInst) animInst->Montage_JumpToSection("End", currentDodgeMont);
	animInst->Montage_Resume(currentDodgeMont);
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
	
	if (HitData.bArmorBreaker && !bCanBlockArmorBreaker) HitData.resolvedReaction = Tags::StateMachine::Action::Reaction::BlockBreak;
	else if (bPerfectBlockWindow) HandlePerfectBlock(HitData);
	else if (combatResComp->IsImmune()) HitData.resolvedReaction = Tags::StateMachine::Action::Reaction::BlockHit; // If immune, just play block hit
	else // Try Block
	{
		++blockCount;
		if (blockCount > maxBlockHits) HitData.resolvedReaction = Tags::StateMachine::Action::Reaction::BlockBreak;
		else HitData.resolvedReaction = Tags::StateMachine::Action::Reaction::BlockHit;
	}

	if (HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::BlockBreak)
	{
		HitData.dmg = 0.0f;
		bBlockBroken = true;
		blockCount = maxBlockHits;
	}
	else if (HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::BlockHit) HitData.dmg = 0.0f;

	// Reset block timers
	FTimerManager& timerManager = world->GetTimerManager();
	timerManager.ClearTimer(TH_BlockRegenDelay);
	timerManager.ClearTimer(TH_BlockRegen);
	timerManager.SetTimer(TH_BlockRegenDelay, this, &UPlayerCombatComponent::StartRegenBlockCount, blockRegenDelay, false);
}