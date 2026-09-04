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
	if (animInst->PlayMontageHNS(AtkData.montage, AtkData.montageSection))
	{
		animInst->Montage_SetEndDelegate(MontageEndedDelegate, AtkData.montage);
		return true;
	}
	else return false;
}

void UEnemyCombatComponent::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (traceComp) traceComp->ClearHitActors(); // Clear all hit actors so they can be hit again

	if (bInterrupted)
	{
		if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, TEXT("[EnemyCombatComp] Attack Montage: Interrupted"));

		// If interrupted by an attack, don't clear because new combat data is often applied by the new attack at this point
		if (iCmbtInst && iCmbtInst->HasTag(Tags::StateMachine::Action::Combat::Attack)) return;
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

void UEnemyCombatComponent::EnterVulnerable()
{
    UWorld* world = GetWorld();
    if (!world || !combatResComp) return;

    FTimerManager& timerManager = world->GetTimerManager();
    if (timerManager.IsTimerActive(TH_Vulnerable)) timerManager.ClearTimer(TH_Vulnerable);

    combatResComp->SetPoiseCalc(-1); // Vulnerable means poise is -1

    timerManager.SetTimer(TH_Vulnerable, this, &UEnemyCombatComponent::ExitVulnerable, vulnerableDuration, false);
}

void UEnemyCombatComponent::ExitVulnerable()
{
    if (UWorld* world = GetWorld())
    {
        FTimerManager& timerManager = world->GetTimerManager();
        if (timerManager.IsTimerActive(TH_Vulnerable)) timerManager.ClearTimer(TH_Vulnerable);
    }

    if (combatResComp) combatResComp->ResetPoiseCalc(); // Reset back to base poise
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

void UEnemyCombatComponent::ReceieveHit(FAtkHitData& HitData)
{
	if (!EnsureReferences() || !combatResComp) return;

	bool bBlocking = iCmbtInst->HasTag(Tags::StateMachine::Action::Combat::Block);

	if (bBlocking) // Blocking
	{
		if (HitData.bArmorBreaker) // Armor breakers always breaks blocks
		{
			HitData.resolvedReaction = Tags::StateMachine::Action::Reaction::BlockBreak;
			if (bHasSuperArmor) // Brake Super Armor
			{
				DeactivateSuperArmor();
				OnSuperArmorBroken.Broadcast();
			}
			EnterVulnerable(); // Broken block makes the enemy become vulnerable
		}
		else HitData.resolvedReaction = Tags::StateMachine::Action::Reaction::BlockHit;

		if (HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::BlockBreak) HitData.dmg = 0.0f;
		else if (HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::BlockHit) HitData.dmg = 0.0f;
	}
	else if (bHasSuperArmor) // Super Armor is active
	{
		if (HitData.bArmorBreaker)
		{
			DeactivateSuperArmor();
			OnSuperArmorBroken.Broadcast();
			EnterVulnerable(); // Broken super armor makes the enemy become vulnerable
		}
		else HitData.resolvedReaction = Tags::StateMachine::Action::Reaction::NoReact; // Super armor not broken, so no hit reaction will be played
	}
	else if (HitData.bIsCounterFollowUp) EnterVulnerable(); // Counter follow-ups cause enemies to become vulnerable
}