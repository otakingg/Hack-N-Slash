#include "PlayerCombatComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../../Interfaces/CharAnimInterface.h"
//#include "../../Interfaces/Damageable.h"
#include "../../Characters/StateMachineComponent.h"

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

    return true;
}

bool UPlayerCombatComponent::IsAtkContextValid(const FPlayerAtkData& AtkData, EPlayerAction PlayerAction, const FVector2D& InputVector) const
{
	UStateMachineComponent* smComp = ownerChar ? ownerChar->FindComponentByClass<UStateMachineComponent>() : nullptr;
	bool bStatesMatch = !AtkData.requiredMovementState.IsValid() || (smComp && smComp->HasExactActiveTag(AtkData.requiredMovementState));

	bool bActionMatch = AtkData.playerAction == PlayerAction;

	bool LStickMotionMatch = true;

	bool bLockRequirementMatch = true;
    return false;
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

	iCharAnimInst->PlayMontageHNS(AtkData->montage);
}

void UPlayerCombatComponent::ClearAtkData() { currentAtkData = nullptr; }

FPlayerAtkData* UPlayerCombatComponent::GetCurrentAtkData() const { return currentAtkData; }