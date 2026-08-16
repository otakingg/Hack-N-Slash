#include "EnemyCombatComponent.h"
#include "GameFramework/Character.h"

#include "../../Animation/AnimInstances/BaseCharAnimInstance.h"
#include "../../Interfaces/CombatInstigator.h"
#include "../Shared/CombatResolutionComponent.h"
#include "../../Combat/Shared/CombatTraceComponent.h"
#include "../../Structs/FAtkHitData.h"
#include "../../Structs/FEnemyAtkData.h"
#include "../../Characters/Shared/LocomotionComponent.h"
#include "../../Characters/Shared/StateMachineComponent.h"

UEnemyCombatComponent::UEnemyCombatComponent() { PrimaryComponentTick.bCanEverTick = false; }

void UEnemyCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	EnsureReferences();
}

void UEnemyCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) { Super::TickComponent(DeltaTime, TickType, ThisTickFunction); }

bool UEnemyCombatComponent::EnsureReferences()
{
    if (!ownerChar) ownerChar = Cast<ACharacter>(GetOwner());
    if (!ownerChar)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UEnemyCombatComponent] Owner is not an ACharacter: %s"), *GetNameSafe(GetOwner()));
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

	if (!stateMachineComp) stateMachineComp = ownerChar ? ownerChar->FindComponentByClass<UStateMachineComponent>() : nullptr;
	if (!stateMachineComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UEnemyCombatComponent] No StateMachineComponent on: %s"), *GetNameSafe(ownerChar));
        return false;
    }

	if (!iCmbtInst) iCmbtInst = Cast<ICombatInstigator>(ownerChar);
	if (!iCmbtInst)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UEnemyCombatComponent] Owner does not implement ICombatInstigator: %s"), *GetNameSafe(ownerChar));
		return false;
	}

	if (!combatResComp) combatResComp = ownerChar ? ownerChar->FindComponentByClass<UCombatResolutionComponent>() : nullptr;
	if (!locoComp) locoComp = ownerChar ? ownerChar->FindComponentByClass<ULocomotionComponent>() : nullptr;
	if (!traceComp) traceComp = ownerChar ? ownerChar->FindComponentByClass<UCombatTraceComponent>() : nullptr;

    return true;
}

bool UEnemyCombatComponent::Attack(const FEnemyAtkData& AtkData)
{
	if (!EnsureReferences() || !AtkData.montage) return false;

	// Try to change to attack state
	if (!stateMachineComp->ChangeActionState(stateMachineComp->GetActionStateByTag(Tags::StateMachine::Action::Combat::Attack), false)) return false;

	FOnMontageEnded MontageEndedDelegate;
	MontageEndedDelegate.BindUObject(this, &UEnemyCombatComponent::OnAttackMontageEnded);
	if (!animInst->PlayMontageHNS(AtkData.montage, AtkData.montageSection))
	{
		stateMachineComp->ClearActionState();
		if (locoComp) locoComp->ClearWarpData();
		if (traceComp) traceComp->ClearHitActors();
		return false;
	}
	animInst->Montage_SetEndDelegate(MontageEndedDelegate, AtkData.montage);
	return true;
}

void UEnemyCombatComponent::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!EnsureReferences()) return;

	if (traceComp) traceComp->ClearHitActors();

	if (bInterrupted)
	{
		if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, TEXT("[EnemyCombatComp] Attack Montage: Interrupted"));
		if (iCmbtInst->HasTag(Tags::StateMachine::Action::Combat::Attack)) return; // If still in attack state, don't clear motion warp data because new warp data is often applied at this point
	}
	else if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, TEXT("[EnemyCombatComp] Attack Montage: Finished"));

	if (locoComp) locoComp->ClearWarpData();
}

bool UEnemyCombatComponent::BlockStart()
{
	if (!EnsureReferences()) return false;

	// Try to change to block state
	if (!stateMachineComp->ChangeActionState(stateMachineComp->GetActionStateByTag(Tags::StateMachine::Action::Combat::Block), false)) return false;
	animInst->StopAllMontages(0.25f);
	return true;
}

void UEnemyCombatComponent::BlockStop()
{
	if (!EnsureReferences()) return;
	stateMachineComp->ClearActionState();
}

void UEnemyCombatComponent::ReceieveHit(FAtkHitData& HitData)
{
	if (!EnsureReferences() || !combatResComp) return;

	bool bBlocking = iCmbtInst->HasTag(Tags::StateMachine::Action::Combat::Block);
	bool bIsImmune = combatResComp->GetVulnerability() == ECombatVulnerability::Immune;

	if (bBlocking)
	{
		if (HitData.bArmorBreaker && !bIsImmune)
		{
			HitData.resolvedReaction = Tags::StateMachine::Action::Reaction::BlockBreak;
			if (bHasSuperArmor)
			{
				DeactivateSuperArmor();
				OnSuperArmorBroken.Broadcast();
			}
			combatResComp->EnterVulnerable();
		}
		else HitData.resolvedReaction = Tags::StateMachine::Action::Reaction::BlockHit;

		if (HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::BlockBreak) HitData.dmg /= 2.0f;
		else HitData.dmg = 0.0f;
	}
	else if (bHasSuperArmor && HitData.bArmorBreaker && !bIsImmune)
	{
		DeactivateSuperArmor();
		OnSuperArmorBroken.Broadcast();
		combatResComp->EnterVulnerable();
	}
}

void UEnemyCombatComponent::ActivateSuperArmor()
{
    if (bHasSuperArmor) return;
    bHasSuperArmor = true;
    OnSuperArmorActivated.Broadcast();
}

void UEnemyCombatComponent::DeactivateSuperArmor()
{
    if (!bHasSuperArmor) return;
    bHasSuperArmor = false;
    OnSuperArmorDeactivated.Broadcast();
}