#include "EnemyCombatComponent.h"
#include "GameFramework/Character.h"

#include "../../Tags/CharacterStateTagNamespaces.h"
#include "../../Interfaces/CharAnimInterface.h"
#include "../Shared/CombatResolutionComponent.h"
#include "../../Combat/Shared/CombatTraceComponent.h"
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

	if (!combatResComp) combatResComp = ownerChar ? ownerChar->FindComponentByClass<UCombatResolutionComponent>() : nullptr;
	if (!stateMachineComp) stateMachineComp = ownerChar ? ownerChar->FindComponentByClass<UStateMachineComponent>() : nullptr;
	if (!traceComp) traceComp = ownerChar ? ownerChar->FindComponentByClass<UCombatTraceComponent>() : nullptr;

    return true;
}

void UEnemyCombatComponent::AttackIntent(const FEnemyAtkData& AtkData)
{
	if (!AtkData.montage) return;

	AActor* target = nullptr;
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

	//FOnMontageEnded MontageEndedDelegate;
	//MontageEndedDelegate.BindUObject(this, &UEnemyCombatComponent::OnAttackMontageEnded);
	iCharAnimInst->PlayMontageHNS(AtkData.montage);
	//iCharAnimInst->SetMontageEndDelegate(MontageEndedDelegate, AtkData.montage);
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