#include "EnemyCombatComponent.h"
#include "GameFramework/Character.h"

#include "../../Animation/AnimInstances/BaseCharAnimInstance.h"
#include "../../Tags/CharacterStateTags.h"
#include "../Shared/CombatResolutionComponent.h"
#include "../../Combat/Shared/CombatTraceComponent.h"
#include "../../Characters/Enemy/EnemyBrainComponent.h"
#include "../../Structs/FAtkHitData.h"
#include "../../Structs/FEnemyAtkData.h"
#include "../../Characters/Shared/LocomotionComponent.h"
#include "../../Characters/Shared/StateMachineComponent.h"

UEnemyCombatComponent::UEnemyCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UEnemyCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	EnsureReferences();
}

void UEnemyCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UEnemyCombatComponent::EnsureReferences()
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

	if (!locoComp) locoComp = ownerChar ? ownerChar->FindComponentByClass<ULocomotionComponent>() : nullptr;
	if (!enemyBrainComp) enemyBrainComp = ownerChar ? ownerChar->FindComponentByClass<UEnemyBrainComponent>() : nullptr;
	if (!combatResComp) combatResComp = ownerChar ? ownerChar->FindComponentByClass<UCombatResolutionComponent>() : nullptr;
	if (!stateMachineComp) stateMachineComp = ownerChar ? ownerChar->FindComponentByClass<UStateMachineComponent>() : nullptr;
	if (!traceComp) traceComp = ownerChar ? ownerChar->FindComponentByClass<UCombatTraceComponent>() : nullptr;

    return true;
}

void UEnemyCombatComponent::Attack(const FEnemyAtkData& AtkData)
{
	if (!EnsureReferences() || !AtkData.montage) return;

	// Try to change to attack state
	if (stateMachineComp && !stateMachineComp->ChangeActionState(stateMachineComp->GetActionStateByTag(StateCombatTags::Attack), false)) return;

	AActor* target = enemyBrainComp ? enemyBrainComp->blackboard.TargetActor : nullptr;
	if (target && locoComp)
	{
		FVector desiredLoc;
		FRotator desiredRot;
		locoComp->GetWarpingLocRot(target, desiredLoc, desiredRot, AtkData.warpOffset, enemyBrainComp->blackboard.bLockedOn);
		locoComp->UpdateMotionWarpData(desiredLoc, desiredRot);
	}

	FOnMontageEnded MontageEndedDelegate;
	MontageEndedDelegate.BindUObject(this, &UEnemyCombatComponent::OnAttackMontageEnded);
	animInst->PlayMontageHNS(AtkData.montage, AtkData.montageSection);
	if (!animInst->PlayMontageHNS(AtkData.montage, AtkData.montageSection))
	{
		stateMachineComp->ClearActionState();
		if (traceComp) traceComp->ClearHitActors();
		if (locoComp) locoComp->ClearMotionWarpData();
		return;
	}
	animInst->Montage_SetEndDelegate(MontageEndedDelegate, AtkData.montage);
}

void UEnemyCombatComponent::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!EnsureReferences()) return;

	if (traceComp) traceComp->ClearHitActors();

	if (bInterrupted)
	{
		if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("[EnemyCombatComp] Attack Montage: Interrupted"));
		if (stateMachineComp && !stateMachineComp->HasActiveTag(StateCombatTags::Attack))
		{
			// New warp data is often set before this when an attack is interrupting, only clear if not interrupting with an attack
			if (locoComp) locoComp->ClearMotionWarpData();
		}
	}
	else
	{
		if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("[EnemyCombatComp] Attack Montage: Finished"));
		if (locoComp) locoComp->ClearMotionWarpData();
	}
}

void UEnemyCombatComponent::BlockStart()
{
	if (!EnsureReferences() || !stateMachineComp || !animInst) return;

	// Try to change to block state
	// If not in block state after attempt, return early because we're not allowed to block
	if (!stateMachineComp->ChangeActionState(stateMachineComp->GetActionStateByTag(StateCombatTags::Block), false)) return;
	animInst->StopAllMontages(0.25f);
}

void UEnemyCombatComponent::BlockStop()
{
	if (!EnsureReferences() || !stateMachineComp) return;
	stateMachineComp->ClearActionState();
}

void UEnemyCombatComponent::ReceieveHit_Implementation(FAtkHitData& HitData)
{
	if (!EnsureReferences() || !combatResComp) return;

	bool bBlocking = stateMachineComp && stateMachineComp->HasExactActiveTag(StateCombatTags::Block);
	bool bIsImmune = combatResComp->GetVulnerability() == ECombatVulnerability::Immune;

	if (bBlocking)
	{
		if (HitData.bArmorBreaker && !bIsImmune)
		{
			HitData.resolvedReaction = StateReactionTags::BlockBreak;
			if (bHasSuperArmor)
			{
				DeactivateSuperArmor();
				OnSuperArmorBroken.Broadcast();
			}
			combatResComp->EnterVulnerable();
		}
		else HitData.resolvedReaction = StateReactionTags::BlockHit;

		if (HitData.resolvedReaction == StateReactionTags::BlockBreak)
		{
			HitData.dmgHP /= 2.0f;
			OnBlockBreak.Broadcast(HitData);
		}
		else
		{
			HitData.dmgHP = 0.0f; // Blocked the hit, so take no damage
			OnBlock.Broadcast(HitData);
		}
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