#include "EnemyCombatComponent.h"
#include "GameFramework/Character.h"

#include "../../Tags/CharacterStateTagNamespaces.h"
#include "../../Interfaces/CharAnimInterface.h"
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

	if (!iCharAnimInst)
	{
		if (USkeletalMeshComponent* skeletalMeshComp = ownerChar->GetMesh()) iCharAnimInst = Cast<ICharAnimInterface>(skeletalMeshComp->GetAnimInstance());
	}
	if (!iCharAnimInst)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UPlayerCombatComponent] Owner's skeletal mesh does not have a valid animation instance that implements ICharAnimInterface: %s"), *GetNameSafe(ownerChar));
		return false;
	}

	if (!enemyBrainComp) enemyBrainComp = ownerChar ? ownerChar->FindComponentByClass<UEnemyBrainComponent>() : nullptr;
	if (!combatResComp) combatResComp = ownerChar ? ownerChar->FindComponentByClass<UCombatResolutionComponent>() : nullptr;
	if (!stateMachineComp) stateMachineComp = ownerChar ? ownerChar->FindComponentByClass<UStateMachineComponent>() : nullptr;
	if (!traceComp) traceComp = ownerChar ? ownerChar->FindComponentByClass<UCombatTraceComponent>() : nullptr;

    return true;
}

void UEnemyCombatComponent::AttackIntent(const FEnemyAtkData &AtkData)
{
	if (!EnsureReferences() || !AtkData.montage) return;

	AActor* target = enemyBrainComp ? enemyBrainComp->blackboard.TargetActor : nullptr;
	if (target)
	{
		FVector desiredLoc;
		FRotator desiredRot;

		ILocomotionCmdInterface* iLocoCmd = stateMachineComp->GetLocomotionCommands();
		if (iLocoCmd)
		{
			iLocoCmd->GetWarpingLocRot(target, desiredLoc, desiredRot, AtkData.warpOffset, "Enemy Combat Comp");
			iLocoCmd->UpdateMotionWarpData(desiredLoc, desiredRot);
		}
	}
	else if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("[EnemyCombatComp] Target is null"));

	//IDamageable* iDmgblTarget = Cast<IDamageable>(target);
	//if (iDmgblTarget) iDmgblTarget->AttackDetected();

	if (stateMachineComp) stateMachineComp->ChangeActionState(stateMachineComp->GetActionStateByTag(CombatTags::Attack), false);

	FOnMontageEnded MontageEndedDelegate;
	MontageEndedDelegate.BindUObject(this, &UEnemyCombatComponent::OnAttackMontageEnded);
	iCharAnimInst->PlayMontageHNS(AtkData.montage);
	iCharAnimInst->SetMontageEndDelegate(MontageEndedDelegate, AtkData.montage);
}

void UEnemyCombatComponent::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!EnsureReferences()) return;

	if (traceComp) traceComp->ClearHitActors();

	if (bInterrupted)
	{
		if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("[EnemyCombatComp] Attack Montage: Interrupted"));
		if (stateMachineComp && !stateMachineComp->IsInActionTag(CombatTags::Attack))
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
	if (!EnsureReferences() || !stateMachineComp || !iCharAnimInst) return;

	iCharAnimInst->StopAllMontagesHNS(0.25f);
	stateMachineComp->ChangeActionState(stateMachineComp->GetActionStateByTag(CombatTags::Block), false); 
}

void UEnemyCombatComponent::BlockStopIntent()
{
	if (!EnsureReferences() || !stateMachineComp) return;
	stateMachineComp->ClearActionState();
}

void UEnemyCombatComponent::ReceieveHit_Implementation(FAtkHitData& HitData)
{
	if (!EnsureReferences() || !combatResComp) return;

	bool bBlocking = stateMachineComp && stateMachineComp->HasExactActiveTag(CombatTags::Block);
	bool bIsImmune = combatResComp->GetVulnerability() == ECombatVulnerability::Immune;

	if (bBlocking)
	{
		if (bHasSuperArmor && HitData.bArmorBreaker && !bIsImmune)
		{
			HitData.resolvedReaction = HitTags::BlockBreak;
			DeactivateSuperArmor();
			OnSuperArmorBroken.Broadcast();
			combatResComp->EnterVulnerable();
		}
		else HitData.resolvedReaction = HitTags::BlockHit;

		if (HitData.resolvedReaction == HitTags::BlockBreak) HitData.dmgHP /= 2.0f;
		else HitData.dmgHP = 0.0f; // Blocked the hit, so take no damage
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