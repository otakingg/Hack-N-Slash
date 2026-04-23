#include "PlayerCombatComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

#include "../../Tags/CharacterStateTagNamespaces.h"
#include "../../Interfaces/CharAnimInterface.h"
#include "../../Combat/Shared/CombatTraceComponent.h"
//#include "../../Interfaces/Damageable.h"
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
}

void UPlayerCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UPlayerCombatComponent::EnsureReferences()
{
    if (!ownerChar) ownerChar = Cast<ACharacter>(GetOwner());
    if (!ownerChar)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UPlayerCombatComponent] Owner is not an ACharacter: %s"), *GetNameSafe(GetOwner()));
        return false;
    }

	if (!iCharAnimInst)
	{
		if (USkeletalMeshComponent* skeletalMeshComp = ownerChar->GetMesh()) iCharAnimInst = Cast<ICharAnimInterface>(skeletalMeshComp->GetAnimInstance());
	}
	if (!iCharAnimInst)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UPlayerCombatComponent] Owner's skeletal mesh does not have a valid animation instance that implements ICharAnimInterface: %s"), *GetNameSafe(ownerChar));
		return false;
	}

    if (!moveComp) moveComp = ownerChar->GetCharacterMovement();
    if (!moveComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UPlayerCombatComponent] No CharacterMovementComponent on: %s"), *GetNameSafe(ownerChar));
        return false;
    }

	if (!playerTargettingComp) playerTargettingComp = ownerChar ? ownerChar->FindComponentByClass<UPlayerTargettingComponent>() : nullptr;
	if (!stateMachineComp) stateMachineComp = ownerChar ? ownerChar->FindComponentByClass<UStateMachineComponent>() : nullptr;
	if (!traceComp) traceComp = ownerChar ? ownerChar->FindComponentByClass<UCombatTraceComponent>() : nullptr;

    return true;
}

EStickMotion UPlayerCombatComponent::GetStickMotion(const FPlayerAtkData& AtkData, const FVector2D& InputVector, AActor* Target) const
{
    if (AtkData.lStickMotion == EStickMotion::Any) return EStickMotion::Any;
    else if (AtkData.lStickMotion == EStickMotion::Neutral && InputVector.IsNearlyZero()) return EStickMotion::Neutral;
    else if (AtkData.lStickMotion == EStickMotion::NotNeutral && !InputVector.IsNearlyZero()) return EStickMotion::NotNeutral;

    FVector forward;
    FVector right;

    if (Target && playerTargettingComp->GetLockedOn())
    {
        forward = Target->GetActorLocation() - ownerChar->GetActorLocation();
        forward.Z = 0.f;
        forward.Normalize();

        right = FVector::CrossProduct(FVector::UpVector, forward).GetSafeNormal();
    }
    else
    {
        const FRotator controlRot = ownerChar->GetControlRotation();
        const FRotator yawOnlyRot(0.f, controlRot.Yaw, 0.f);

        forward = FRotationMatrix(yawOnlyRot).GetUnitAxis(EAxis::X);
        right   = FRotationMatrix(yawOnlyRot).GetUnitAxis(EAxis::Y);
    }

    FVector moveDir = (forward * InputVector.Y) + (right * InputVector.X);
    if (moveDir.IsNearlyZero()) return EStickMotion::Neutral;
    moveDir.Normalize();

    const float forwardDot = FVector::DotProduct(moveDir, forward);
    const float rightDot   = FVector::DotProduct(moveDir, right);

    if (FMath::Abs(forwardDot) > FMath::Abs(rightDot)) return (forwardDot > 0.0f) ? EStickMotion::Forward : EStickMotion::Back;
    else 											   return (rightDot > 0.0f) ? EStickMotion::Right : EStickMotion::Left;
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
	AActor* target = playerTargettingComp ? playerTargettingComp->GetCurrentTarget() : nullptr;
	EStickMotion lStickMotion = GetStickMotion(AtkData, InputVector, target);
	bLStickMotionMatch = AtkData.lStickMotion == lStickMotion;


	bool bStatesMatch = !AtkData.requiredMovementState.IsValid() || (stateMachineComp && stateMachineComp->HasExactActiveTag(AtkData.requiredMovementState));
	
    return bStatesMatch && bActionMatch && bLStickMotionMatch && bLockRequirementMatch;
}

void UPlayerCombatComponent::AttackIntent(const FVector2D& Dir, EPlayerAction PlayerAction)
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
			return;
		}
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
			return;
		}
	}

	PerformAttack(nextAtkData, InputVector);
}

void UPlayerCombatComponent::PerformAttack(FPlayerAtkData* AtkData, const FVector2D& Dir)
{
	if (!AtkData || !AtkData->montage) return;

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
			if (AtkData->bIgnoreFreeFlowRules) iLocoCmd->GetWarpingLocRot(target, desiredLoc, desiredRot, AtkData->warpOffset, "Hello");
			else iLocoCmd->GetWarpingLocRot(target, desiredLoc, desiredRot, AtkData->warpOffset, Dir, playerTargettingComp->GetLockedOn());
			iLocoCmd->UpdateMotionWarpData(desiredLoc, desiredRot);
		}
	}
	else
	{
		if (bDebug && GEngine) {GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("[PlayerCombatComp] Target is null"));}
		if (!Dir.IsNearlyZero() && (!playerTargettingComp || !playerTargettingComp->GetLockedOn()))
		{
			// Rotate in direction of input if holding a direction
			const FRotator ControlRot = ownerChar->GetControlRotation();
			const FRotator YawRot(0.f, ControlRot.Yaw, 0.f);

			const FVector forward = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
			const FVector right   = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);

			FVector MoveDir = forward * Dir.Y + right * Dir.X;
			MoveDir.Z = 0.f;
			MoveDir.Normalize();

			ownerChar->SetActorRotation(MoveDir.Rotation());
		}
	}

	currentAtkData = AtkData;

	//IDamageable* iDmgblTarget = Cast<IDamageable>(target);
	//if (iDmgblTarget) iDmgblTarget->AttackDetected();

	if (stateMachineComp) stateMachineComp->ChangeActionState(stateMachineComp->GetActionStateByTag(CombatTags::Attack), false);

	FOnMontageEnded MontageEndedDelegate;
	MontageEndedDelegate.BindUObject(this, &UPlayerCombatComponent::OnAttackMontageEnded);
	iCharAnimInst->PlayMontageHNS(AtkData->montage);
	iCharAnimInst->SetMontageEndDelegate(MontageEndedDelegate, AtkData->montage);
}

void UPlayerCombatComponent::OnAttackMontageEnded(UAnimMontage* montage, bool bInterrupted)
{
	if (traceComp) traceComp->ClearHitActors();
	
	if (bInterrupted)
	{
		if (bDebug && GEngine) {GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("[PlayerCombatComp] Attack Montage: Interrupted"));}
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
		if (bDebug && GEngine) {GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("[PlayerCombatComp] Attack Montage: Finished"));}
		if (ILocomotionCmdInterface* iLocoCmd = stateMachineComp->GetLocomotionCommands()) iLocoCmd->ClearMotionWarpData();
		if (playerTargettingComp) playerTargettingComp->ClearCurrentTarget();
		ClearAtkData();
	}
}

void UPlayerCombatComponent::ClearAtkData() { currentAtkData = nullptr; }

FPlayerAtkData* UPlayerCombatComponent::GetCurrentAtkData() const { return currentAtkData; }