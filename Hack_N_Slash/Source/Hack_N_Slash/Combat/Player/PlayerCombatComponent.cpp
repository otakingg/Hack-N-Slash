#include "PlayerCombatComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../../Tags/CharacterStateTagNamespaces.h"
#include "../../Interfaces/CharAnimInterface.h"
#include "../../Combat/Shared/CombatTraceComponent.h"
//#include "../../Interfaces/Damageable.h"
#include "../../Characters/Shared/StateMachineComponent.h"

UPlayerCombatComponent::UPlayerCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	EnsureOwnerCharacter();
}

void UPlayerCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UPlayerCombatComponent::EnsureOwnerCharacter()
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

	if (!stateMachineComp) stateMachineComp = ownerChar ? ownerChar->FindComponentByClass<UStateMachineComponent>() : nullptr;
	if (!traceComp) traceComp = ownerChar ? ownerChar->FindComponentByClass<UCombatTraceComponent>() : nullptr;

    return true;
}

bool UPlayerCombatComponent::IsAtkContextValid(const FPlayerAtkData& AtkData, EPlayerAction PlayerAction, const FVector2D& InputVector) const
{
	bool bStatesMatch = !AtkData.requiredMovementState.IsValid() || (stateMachineComp && stateMachineComp->HasExactActiveTag(AtkData.requiredMovementState));

	bool bActionMatch = AtkData.playerAction == PlayerAction;

	bool LStickMotionMatch = true;

	bool bLockRequirementMatch = true;
    return bStatesMatch && bActionMatch && LStickMotionMatch && bLockRequirementMatch;
}

void UPlayerCombatComponent::AttackIntent(const FVector2D& Dir)
{
	// Later I need to determine wether to choose heavy or light attack
	AttackLightStart(Dir);
}

void UPlayerCombatComponent::AttackHeavyStart(const FVector2D &InputVector)
{
	if (!EnsureOwnerCharacter() || !activeAtkDT) return;
}

void UPlayerCombatComponent::AttackLightStart(const FVector2D &InputVector)
{
	if (!EnsureOwnerCharacter() || !activeAtkDT) return;

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

	PerformAttack(nextAtkData);
}

void UPlayerCombatComponent::PerformAttack(FPlayerAtkData* AtkData)
{
	if (!AtkData || !AtkData->montage) return;
	currentAtkData = AtkData;

	//IDamageable* iDmgblTarget = Cast<IDamageable>(target);
	//if (iDmgblTarget) iDmgblTarget->AttackDetected();

	if (stateMachineComp) stateMachineComp->ChangeActionState(stateMachineComp->GetActionStateByTag(CombatTags::Attack), false);

	FOnMontageEnded MontageEndedDelegate;
	MontageEndedDelegate.BindUObject(this, &UPlayerCombatComponent::OnAttackMontageEnded);
	iCharAnimInst->PlayMontageHNS(AtkData->montage);
	iCharAnimInst->SetMontageEndDelegate(MontageEndedDelegate, AtkData->montage);
}

void UPlayerCombatComponent::OnAttackMontageEnded(UAnimMontage *montage, bool bInterrupted)
{
	if (traceComp) traceComp->ClearHitActors();
	
	if (bInterrupted)
	{
		if (bDebug && GEngine) {GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("[PlayerCombatComp] Attack Montage: Interrupted"));}
		if (stateMachineComp && !stateMachineComp->IsInActionTag(CombatTags::Attack)) ClearAtkData(); // Only clear if not interrupting with another attack so as to not overight the new atk data
	}
	else
	{
		if (bDebug && GEngine) {GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("[PlayerCombatComp] Attack Montage: Finished"));}
		// Don't clear on interrupted because it could lead to unwanted behavior
		// EX: Another attack is interrupting the current attack. Don't want to clear the new attack's attack data
		ClearAtkData();
	}
}

void UPlayerCombatComponent::ClearAtkData() { currentAtkData = nullptr; }

FPlayerAtkData* UPlayerCombatComponent::GetCurrentAtkData() const { return currentAtkData; }