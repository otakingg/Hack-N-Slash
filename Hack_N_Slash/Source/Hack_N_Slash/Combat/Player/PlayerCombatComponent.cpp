#include "PlayerCombatComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/RootMotionSource.h"

#include "../../Animation/AnimInstances/BaseCharAnimInstance.h"
#include "../../Tags/CharacterStateTagNamespaces.h"
#include "../Shared/CombatResolutionComponent.h"
#include "../../Combat/Shared/CombatTraceComponent.h"
#include "../../Interfaces/Damageable.h"
#include "../Enemy/EnemyCombatComponent.h"
#include "../../Structs/FAtkHitData.h"
#include "../../Interfaces/LocomotionCmdInterface.h"
#include "../../Combat/Player/PlayerTargettingComponent.h"
#include "../../Characters/Shared/StateMachineComponent.h"

UPlayerCombatComponent::UPlayerCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	EnsureReferences();
	if (ownerChar) ownerChar->LandedDelegate.AddDynamic(this, &UPlayerCombatComponent::HandleLanded);
}

void UPlayerCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
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
		UE_LOG(LogTemp, Warning, TEXT("[UEnemyCombatComponent] Owner's skeletal mesh does not have a valid base char animation instance: %s"), *GetNameSafe(ownerChar));
		return false;
	}


    if (!moveComp) moveComp = ownerChar->GetCharacterMovement();
    if (!moveComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UPlayerCombatComponent] No CharacterMovementComponent on: %s"), *GetNameSafe(ownerChar));
        return false;
    }

	if (!combatResComp) combatResComp = ownerChar ? ownerChar->FindComponentByClass<UCombatResolutionComponent>() : nullptr;
	if (!playerTargettingComp) playerTargettingComp = ownerChar ? ownerChar->FindComponentByClass<UPlayerTargettingComponent>() : nullptr;
	if (!stateMachineComp) stateMachineComp = ownerChar ? ownerChar->FindComponentByClass<UStateMachineComponent>() : nullptr;
	if (!traceComp) traceComp = ownerChar ? ownerChar->FindComponentByClass<UCombatTraceComponent>() : nullptr;

    return true;
}

void UPlayerCombatComponent::ClearAtkData() { currentAtkData = nullptr; }
FPlayerAtkData* UPlayerCombatComponent::GetCurrentAtkData() const { return currentAtkData; }

FVector UPlayerCombatComponent::GetInputWorldDirRelativeToCamOrTarget(const FVector2D& InputVector, FVector& OutLocalForward, FVector& OutLocalRight, AActor* Target) const
{
	if (InputVector.IsNearlyZero()) return FVector::ZeroVector;

    if (Target && playerTargettingComp->GetLockedOn()) // Calculate direction relative to the target
    {
        OutLocalForward = Target->GetActorLocation() - ownerChar->GetActorLocation(); // The new "forward"
        OutLocalForward.Z = 0.f; // Flatten to horizontal plane
        OutLocalForward.Normalize(); // Normalize because we only care about direction

		// Cross product generates a perpendicular direction
		// Gives the right by crossing up with forward
        OutLocalRight = FVector::CrossProduct(FVector::UpVector, OutLocalForward).GetSafeNormal();
    }
    else // Calculate direction relative to the camera
    {
		// Only use camera yaw
        const FRotator controlRot = ownerChar->GetControlRotation();
        const FRotator yawOnlyRot(0.f, controlRot.Yaw, 0.f);

        OutLocalForward = FRotationMatrix(yawOnlyRot).GetUnitAxis(EAxis::X); // Camera forward
        OutLocalRight   = FRotationMatrix(yawOnlyRot).GetUnitAxis(EAxis::Y); // Camera right
    }

	FVector inputWorldDir = (OutLocalForward * InputVector.Y) + (OutLocalRight * InputVector.X); // Converts stick input into world direction
	inputWorldDir.Z = 0.f;
	return inputWorldDir.GetSafeNormal(); // Normalize because we only care about direction
}

EStickMotion UPlayerCombatComponent::GetStickMotionFromWorldDir(const FVector& WorldDir, const FVector& LocalForward, const FVector& LocalRight) const
{
	if (WorldDir.IsNearlyZero() || LocalForward.IsNearlyZero() || LocalRight.IsNearlyZero()) return EStickMotion::Neutral;

	FVector moveDir = WorldDir;
	moveDir.Z = 0.f;
	moveDir = moveDir.GetSafeNormal();

	FVector forward = LocalForward;
	forward.Z = 0.f;
	forward = forward.GetSafeNormal();

	FVector right = LocalRight;
	right.Z = 0.f;
	right = right.GetSafeNormal();

	const float forwardDot = FVector::DotProduct(moveDir, forward);
	const float rightDot   = FVector::DotProduct(moveDir, right);

	// Measure Direction Alignment
	// atan2(right, forward): 0 = forward, 90 = right, 180/-180 = back, -90 = left
	const float angleRad = FMath::Atan2(rightDot, forwardDot);
	float angleDeg = FMath::RadiansToDegrees(angleRad);

	// Normalize to [0, 360)
	if (angleDeg < 0.f) angleDeg += 360.f;

	// 8-way sectors, 45 degrees each
	if (angleDeg >= 337.5f || angleDeg < 22.5f) return EStickMotion::Forward;
	else if (angleDeg < 67.5f)   return EStickMotion::ForwardRight;
	else if (angleDeg < 112.5f)  return EStickMotion::Right;
	else if (angleDeg < 157.5f)  return EStickMotion::BackRight;
	else if (angleDeg < 202.5f)  return EStickMotion::Back;
	else if (angleDeg < 247.5f)  return EStickMotion::BackLeft;
	else if (angleDeg < 292.5f)  return EStickMotion::Left;
	else return EStickMotion::ForwardLeft;
}

EStickMotion UPlayerCombatComponent::GetWorldDirRelativeToPlayerFacing(const FVector& WorldDir) const
{
	if (WorldDir.IsNearlyZero()) return EStickMotion::Neutral;

	FVector playerForward = ownerChar->GetActorForwardVector();
	playerForward.Z = 0.f;
	playerForward = playerForward.GetSafeNormal();

	FVector playerRight = ownerChar->GetActorRightVector();
	playerRight.Z = 0.f;
	playerRight = playerRight.GetSafeNormal();

	return GetStickMotionFromWorldDir(WorldDir, playerForward, playerRight);
}

bool UPlayerCombatComponent::IsAtkContextValid(const FPlayerAtkData& AtkData, EPlayerAction PlayerAction, const FVector2D& InputVector) const
{
	bool bActionMatch = AtkData.playerAction == PlayerAction;

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

	bool bLStickMotionMatch = false;

    if (AtkData.lStickMotion == EStickMotion::Any) bLStickMotionMatch = true;
    else if (AtkData.lStickMotion == EStickMotion::Neutral) bLStickMotionMatch = InputVector.IsNearlyZero();
    else if (AtkData.lStickMotion == EStickMotion::NotNeutral) bLStickMotionMatch = !InputVector.IsNearlyZero();
	else
	{
		AActor* target = playerTargettingComp ? playerTargettingComp->GetCurrentTarget() : nullptr;
		FVector localForward, localRight;
		FVector InputWorldDir = GetInputWorldDirRelativeToCamOrTarget(InputVector, localForward, localRight, target);
		EStickMotion lStickMotion = GetStickMotionFromWorldDir(InputWorldDir, localForward, localRight);
		bLStickMotionMatch = AtkData.lStickMotion == lStickMotion;
	}


	bool bStatesMatch = !AtkData.requiredMovementState.IsValid() || (stateMachineComp && stateMachineComp->HasExactActiveTag(AtkData.requiredMovementState));
	
    return bActionMatch && bLockRequirementMatch && bLStickMotionMatch && bStatesMatch;
}

void UPlayerCombatComponent::SnapToInputDirection(const FVector2D& InputDir)
{
	// Rotate in direction of input if holding a direction
	const FRotator ControlRot = ownerChar->GetControlRotation();
	const FRotator YawRot(0.f, ControlRot.Yaw, 0.f);

	const FVector forward = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
	const FVector right   = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);

	FVector MoveDir = forward * InputDir.Y + right * InputDir.X;
	MoveDir.Z = 0.f;
	MoveDir.Normalize();

	ownerChar->SetActorRotation(MoveDir.Rotation());
}

void UPlayerCombatComponent::AttackIntent(const FVector2D &Dir, EPlayerAction PlayerAction)
{
	switch (PlayerAction)
	{
	case EPlayerAction::AttackHeavyStart:
		AttackHeavyStart(Dir);
		break;

	case EPlayerAction::AttackLightStart:
		AttackLightStart(Dir);
		break;
	
	default:
		break;
	}
}

void UPlayerCombatComponent::AttackHeavyStart(const FVector2D& InputVector)
{
	if (!EnsureReferences() || !activeAtkDT) return;

	FPlayerAtkData* nextAtkData = nullptr;
	if (!currentAtkData)
	{
		static const FString contextStr(TEXT("Finding Atk Data Table From 'Attack Heavy Start'. Getting Initial Attack"));
		TArray<FName> rowNames = activeAtkDT->GetRowNames();
		for (FName row : rowNames)
		{
			FPlayerAtkData* rowData = activeAtkDT->FindRow<FPlayerAtkData>(row, contextStr);
			if (!rowData) {continue;}

			if (IsAtkContextValid(*rowData, EPlayerAction::AttackHeavyStart, InputVector))
			{
				nextAtkData = rowData;
				break;
			}
		}
	}
	else
	{
		static const FString contextStr(TEXT("Finding Atk Data Table From 'Attack Heavy Start'. Getting Next Attack"));
		TArray<FName> rowNames = activeAtkDT->GetRowNames();
		for (FName row : rowNames)
		{
			FPlayerAtkData* rowData = activeAtkDT->FindRow<FPlayerAtkData>(row, contextStr);
			if (!rowData) {continue;}

			if (IsAtkContextValid(*rowData, EPlayerAction::AttackHeavyStart, InputVector))
			{
				nextAtkData = rowData;
				break;
			}
		}
	}

	if (!nextAtkData)
	{
		if (bDebug)
		{
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("[UPlayerCombatComponent] [Attack Heavy Start] No valid attack found for input"));
			UE_LOG(LogTemp, Warning, TEXT("[UPlayerCombatComponent] [Attack Heavy Start] No valid attack found for input"));
		}
		return;
	}

	PerformAttack(nextAtkData, InputVector);
}

void UPlayerCombatComponent::AttackLightStart(const FVector2D& InputVector)
{
	if (!EnsureReferences() || !activeAtkDT) return;

	FPlayerAtkData* nextAtkData = nullptr;
	if (!currentAtkData)
	{
		static const FString contextStr(TEXT("Finding Atk Data Table From 'Attack Light Start'. Getting Initial Attack"));
		TArray<FName> rowNames = activeAtkDT->GetRowNames();
		for (FName row : rowNames)
		{
			FPlayerAtkData* rowData = activeAtkDT->FindRow<FPlayerAtkData>(row, contextStr);
			if (!rowData) {continue;}

			if (IsAtkContextValid(*rowData, EPlayerAction::AttackLightStart, InputVector))
			{
				nextAtkData = rowData;
				break;
			}
		}
	}
	else
	{
		static const FString contextStr(TEXT("Finding Atk Data Table From 'Attack Light Start'. Getting Next Attack"));
		TArray<FName> rowNames = activeAtkDT->GetRowNames();
		for (FName row : rowNames)
		{
			FPlayerAtkData* rowData = activeAtkDT->FindRow<FPlayerAtkData>(row, contextStr);
			if (!rowData) {continue;}

			if (IsAtkContextValid(*rowData, EPlayerAction::AttackLightStart, InputVector))
			{
				nextAtkData = rowData;
				break;
			}
		}
	}

	if (!nextAtkData)
	{
		if (bDebug)
		{
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("[UPlayerCombatComponent] [Attack Light Start] No valid attack found for input"));
			UE_LOG(LogTemp, Warning, TEXT("[UPlayerCombatComponent] [Attack Light Start] No valid attack found for input"));
		}
		return;
	}

	PerformAttack(nextAtkData, InputVector);
}

void UPlayerCombatComponent::PerformAttack(FPlayerAtkData* AtkData, const FVector2D& Dir)
{
	if (!AtkData || !AtkData->montage) return;

	bool bAirborne = (stateMachineComp && stateMachineComp->IsAirborne()) || (moveComp && moveComp->IsFalling());
	if (bAirborne)
	{
		if (!bCanAirAtk)
		{
			if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("[PlayerCombatComp] Cannot attack in the air"));
			return;
		}
		else bHasAirAttacked = true;
	}

	AActor* target = nullptr;
	if (playerTargettingComp)
	{
		playerTargettingComp->SoftTarget(Dir);
		target = playerTargettingComp->GetCurrentTarget();
	}

	if (target)
	{
		FVector desiredLoc;
		FRotator desiredRot;

		ILocomotionCmdInterface* iLocoCmd = stateMachineComp->GetLocomotionCommands();
		if (iLocoCmd)
		{
			if (AtkData->bIgnoreFreeFlowRules) iLocoCmd->GetWarpingLocRot(target, desiredLoc, desiredRot, AtkData->warpOffset, "Player Combat Comp");
			else iLocoCmd->GetWarpingLocRot(target, desiredLoc, desiredRot, AtkData->warpOffset, Dir, playerTargettingComp->GetLockedOn());
			iLocoCmd->UpdateMotionWarpData(desiredLoc, desiredRot);
		}
	}
	else
	{
		if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("[PlayerCombatComp] Target is null"));
		if (!Dir.IsNearlyZero() && (!playerTargettingComp || !playerTargettingComp->GetLockedOn())) SnapToInputDirection(Dir);
	}

	currentAtkData = AtkData;

	//IDamageable* iDmgblTarget = Cast<IDamageable>(target);
	//if (iDmgblTarget) iDmgblTarget->AttackDetected();

	if (stateMachineComp) stateMachineComp->ChangeActionState(stateMachineComp->GetActionStateByTag(CombatTags::Attack), false);

	FOnMontageEnded MontageEndedDelegate;
	MontageEndedDelegate.BindUObject(this, &UPlayerCombatComponent::OnAttackMontageEnded);
	animInst->PlayMontageHNS(AtkData->montage);
	animInst->Montage_SetEndDelegate(MontageEndedDelegate, AtkData->montage);
}

void UPlayerCombatComponent::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!EnsureReferences()) return;
	
	if (traceComp) traceComp->ClearHitActors();
	
	if (bInterrupted)
	{
		if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("[PlayerCombatComp] Attack Montage: Interrupted"));
		if (stateMachineComp && !stateMachineComp->IsInActionTag(CombatTags::Attack))
		{
			ClearAtkData(); // Only clear if not interrupting with another attack so as to not overight the new atk data
			// Only clear if not interrupting with another atk so as to not mess with the targetting info
			// This often occurs after a new target and warp data are set, so this is necessary
			if (ILocomotionCmdInterface* iLocoCmd = stateMachineComp->GetLocomotionCommands()) iLocoCmd->ClearMotionWarpData();
			if (playerTargettingComp) playerTargettingComp->ClearCurrentTarget();
		}
	}
	else
	{
		if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("[PlayerCombatComp] Attack Montage: Finished"));
		if (ILocomotionCmdInterface* iLocoCmd = stateMachineComp->GetLocomotionCommands()) iLocoCmd->ClearMotionWarpData();
		if (playerTargettingComp) playerTargettingComp->ClearCurrentTarget();
		ClearAtkData();
	}
}

void UPlayerCombatComponent::BlockStartIntent()
{
	if (!EnsureReferences() || bBlockBroken || !stateMachineComp || !stateMachineComp->IsGrounded() || !animInst) return;

	animInst->StopAllMontages(0.25f);
	stateMachineComp->ChangeActionState(stateMachineComp->GetActionStateByTag(CombatTags::Block), false); 
}

void UPlayerCombatComponent::BlockStopIntent()
{
	if (!EnsureReferences() || !stateMachineComp) return;
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
	if (stateMachineComp && stateMachineComp->HasActiveTag(CombatTags::Block)) return;

	--blockCount;
	blockCount = FMath::Clamp(blockCount, 0, maxBlockHits);

	if (blockCount <= 0) if (UWorld* world = GetWorld()) world->GetTimerManager().ClearTimer(TH_BlockRegen);
}

void UPlayerCombatComponent::DodgeIntent(const FVector2D& Dir)
{
	if (!EnsureReferences()) return;

	UWorld* world = GetWorld();
	if (!world) return;

	UAnimMontage* dodgeMont = nullptr;
	FVector dodgeForce = FVector::ZeroVector;
	EStickMotion dodgeMotion = EStickMotion::Forward;

	bool bGrounded = stateMachineComp && stateMachineComp->IsGrounded() || moveComp->IsMovingOnGround();
	if (bGrounded)
	{
		AActor* target = playerTargettingComp ? playerTargettingComp->GetCurrentTarget() : nullptr;

		// Step 1: input direction relative to camera / target.
		FVector localForward, localRight;
		const FVector dodgeWorldDir = GetInputWorldDirRelativeToCamOrTarget(Dir, localForward, localRight, target);

		// Step 2: montage direction relative to player facing.
		dodgeMotion = GetWorldDirRelativeToPlayerFacing(dodgeWorldDir);

		dodgeForce = dodgeWorldDir * (distance / duration); // Calculate the necessary force to cover the dodge distance in the desired duration

		switch (dodgeMotion)
		{
		case EStickMotion::Back:
		case EStickMotion::BackLeft:
		case EStickMotion::BackRight:
			dodgeMont = groundDodgeMontBack;
			break;
		
		case EStickMotion::Forward:
		case EStickMotion::ForwardLeft:
		case EStickMotion::ForwardRight:
			dodgeMont = groundDodgeMontFwd;
			break;

		case EStickMotion::Left:
			dodgeMont = groundDodgeMontLeft;
			break;

		case EStickMotion::Right:
			dodgeMont = groundDodgeMontRight;
			break;

		default:
			break;
		}
	}
	else if (airDodgeCount >= maxAirDodges)
	{
		if (bDebug)
		{
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, FString::Printf(TEXT("[UPlayerCombatComponent] Max air dodges reached. Current: %d, Max: %d"), airDodgeCount, maxAirDodges));
			UE_LOG(LogTemp, Warning, TEXT("[UPlayerCombatComponent] Max air dodges reached. Current: %d, Max: %d"), airDodgeCount, maxAirDodges);
		}
		return;
	}
	else
	{
		++airDodgeCount;
		airDodgeCount = FMath::Clamp(airDodgeCount, 0, maxAirDodges);
		dodgeMont = airDodgeMont;
		dodgeForce = ownerChar->GetActorForwardVector() * (distance / duration); // Calculate the necessary force to cover the dodge distance in the desired duration
	}

	ILocomotionCmdInterface* iLocoCmd = stateMachineComp ? stateMachineComp->GetLocomotionCommands() : nullptr;
	if (!iLocoCmd) return;

	if (!animInst->PlayMontageHNS(dodgeMont)) return;
	
	stateMachineComp->ChangeActionState(stateMachineComp->GetActionStateByTag(CombatTags::Dodge), false);

	UAsyncRootMovement* aSyncRootMovement = iLocoCmd->ApplyRootMotionSourceConstant(duration, dodgeForce, setVelocityOnFinish, clampVelocityOnFinish, velocityOnFinishMode, strengthOverTime, bIsAdditive);
	if (!aSyncRootMovement) return;

	aSyncRootMovement->OnComplete.AddDynamic(this, &UPlayerCombatComponent::EndDodge);
}

void UPlayerCombatComponent::EndDodge()
{
	if (!EnsureReferences()) return;

	UAnimMontage* dodgeMont = animInst->GetCurrentActiveMontage();
	if (dodgeMont) animInst->PlayMontageHNS(dodgeMont, FName("End"));
}

void UPlayerCombatComponent::HandleLanded(const FHitResult& Hit)
{
	airDodgeCount = 0;
	bHasAirAttacked = false;
	bCanAirAtk = true;
}

void UPlayerCombatComponent::ReceieveHit(FAtkHitData& HitData)
{
	if (!EnsureReferences() || !combatResComp) return;

	bool bBlocking = stateMachineComp && stateMachineComp->HasExactActiveTag(CombatTags::Block);
	if (!bBlocking) return;

	UWorld* world = GetWorld();
	if (!world) return;

	bool bIsImmune = combatResComp->GetVulnerability() == ECombatVulnerability::Immune;

	UEnemyCombatComponent* enemyCmbtComp = HitData.attacker ? HitData.attacker->FindComponentByClass<UEnemyCombatComponent>() : nullptr;
	bool bAtkerHasSuperArmor = enemyCmbtComp && enemyCmbtComp->HasSuperArmor();
	
	if (bAtkerHasSuperArmor && !bCanBlockSuperArmor && !bIsImmune) HitData.resolvedReaction = ReactionTags::BlockBreak; // If can't attacker has super armor and can't block it, block breaks
	else if (bPerfectBlockWindow) // Perfect Block
	{
		if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, TEXT("[UPlayerCombatComponent] Perfect Block!"));
		OnPerfectBlock.Broadcast();
		HitData.resolvedReaction = ReactionTags::BlockHit;
		blockCount = 0;
		if (IDamageable* iDmgblAtkr = Cast<IDamageable>(HitData.damager)) iDmgblAtkr->Countered(ownerChar, "Perfect Block"); // Tell the damager they were countered
	}
	else if (bIsImmune) HitData.resolvedReaction = ReactionTags::BlockHit; // If immune, just play block hit
	else // Try Block
	{
		++blockCount;
		if (blockCount > maxBlockHits) HitData.resolvedReaction = ReactionTags::BlockBreak;
		else HitData.resolvedReaction = ReactionTags::BlockHit;
	}

	if (HitData.resolvedReaction == ReactionTags::BlockBreak)
	{
		HitData.dmgHP /= 2.0f; // Block broken means take half damage
		bBlockBroken = true;
		blockCount = maxBlockHits;
	}
	else HitData.dmgHP = 0.0f; // Blocked the hit, so take no damage

	FTimerManager& timerManager = world->GetTimerManager();
	timerManager.ClearTimer(TH_BlockRegenDelay);
	timerManager.ClearTimer(TH_BlockRegen);
	timerManager.SetTimer(TH_BlockRegenDelay, this, &UPlayerCombatComponent::StartRegenBlockCount, blockRegenDelay, false);
}