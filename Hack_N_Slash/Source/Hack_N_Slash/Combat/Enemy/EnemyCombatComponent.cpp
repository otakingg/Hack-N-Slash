#include "EnemyCombatComponent.h"
#include "GameFramework/Character.h"

#include "../../Animation/AnimInstances/BaseCharAnimInstance.h"
#include "../../Tags/CharacterStateTags.h"
#include "../Shared/CombatResolutionComponent.h"
#include "../../Combat/Shared/CombatTraceComponent.h"
#include "../../Characters/Enemy/EnemyBrainComponent.h"
#include "../../Structs/FAtkHitData.h"
#include "../../Structs/FEnemyAtkData.h"
#include "../../Interfaces/LocomotionCmdInterface.h"
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

	if (!enemyBrainComp) enemyBrainComp = ownerChar ? ownerChar->FindComponentByClass<UEnemyBrainComponent>() : nullptr;
	if (!combatResComp) combatResComp = ownerChar ? ownerChar->FindComponentByClass<UCombatResolutionComponent>() : nullptr;
	if (!stateMachineComp) stateMachineComp = ownerChar ? ownerChar->FindComponentByClass<UStateMachineComponent>() : nullptr;
	if (!traceComp) traceComp = ownerChar ? ownerChar->FindComponentByClass<UCombatTraceComponent>() : nullptr;

    return true;
}

void UEnemyCombatComponent::AttackIntent(const FEnemyAtkData& AtkData)
{
	if (!EnsureReferences() || !AtkData.montage) return;

	AActor* target = enemyBrainComp ? enemyBrainComp->blackboard.TargetActor : nullptr;
	if (target)
	{
		ILocomotionCmdInterface* iLocoCmd = stateMachineComp->GetLocomotionCommands();
		if (iLocoCmd)
		{
			FVector desiredLoc;
			FRotator desiredRot;
			iLocoCmd->GetWarpingLocRot(target, desiredLoc, desiredRot, AtkData.warpOffset, enemyBrainComp->blackboard.bLockedOn);
			iLocoCmd->UpdateMotionWarpData(desiredLoc, desiredRot);
		}
	}
	else if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("[EnemyCombatComp] Target is null"));

	if (stateMachineComp) stateMachineComp->ChangeActionState(stateMachineComp->GetActionStateByTag(StateCombatTags::Attack), false);

	FOnMontageEnded MontageEndedDelegate;
	MontageEndedDelegate.BindUObject(this, &UEnemyCombatComponent::OnAttackMontageEnded);
	animInst->PlayMontageHNS(AtkData.montage, AtkData.montageSection);
	animInst->Montage_SetEndDelegate(MontageEndedDelegate, AtkData.montage);
}

void UEnemyCombatComponent::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!EnsureReferences()) return;

	if (traceComp) traceComp->ClearHitActors();

	if (bInterrupted)
	{
		if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("[EnemyCombatComp] Attack Montage: Interrupted"));
		if (stateMachineComp && !stateMachineComp->IsInActionTag(StateCombatTags::Attack))
		{
			// New warp data is often set before this when an attack is interrupting, only clear if not interrupting with an attack
			if (ILocomotionCmdInterface* iLocoCmd = stateMachineComp->GetLocomotionCommands()) iLocoCmd->ClearMotionWarpData();
		}
	}
	else
	{
		if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("[EnemyCombatComp] Attack Montage: Finished"));
		if (ILocomotionCmdInterface* iLocoCmd = stateMachineComp->GetLocomotionCommands()) iLocoCmd->ClearMotionWarpData();
	}
}

void UEnemyCombatComponent::BlockStartIntent()
{
	if (!EnsureReferences() || !stateMachineComp || !animInst) return;

	animInst->StopAllMontages(0.25f);
	stateMachineComp->ChangeActionState(stateMachineComp->GetActionStateByTag(StateCombatTags::Block), false); 
}

void UEnemyCombatComponent::BlockStopIntent()
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