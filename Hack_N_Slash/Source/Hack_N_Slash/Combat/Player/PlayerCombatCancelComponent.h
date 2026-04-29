// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "PlayerCombatCancelComponent.generated.h"

class UPlayerCombatComponent;
class UStateMachineComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HACK_N_SLASH_API UPlayerCombatCancelComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	AActor* owner = nullptr;
	UPlayerCombatComponent* combatComp = nullptr;
	UStateMachineComponent* stateMachineComp = nullptr;

protected:
	UPROPERTY(EditAnywhere, Category = "Cancel")
	bool bDebug = false;

	UPROPERTY(VisibleAnywhere, Category = "Cancel")
	bool bCanCancelCurrentAction = false; // Will be set by outside sources (AnimNotifies, StateMachine, etc) to determine whether the current action can be cancelled or not

	UPROPERTY(EditAnywhere, Category = "Cancel|States", meta = (Categories = "State.Action.Combat", ToolTip = "Combat states that dodge can be canceled into"))
	TArray<FGameplayTag> cancelableDodgeStates;

	UPROPERTY(EditAnywhere, Category = "Cancel|States", meta = (Categories = "State.Action.Combat", ToolTip = "Combat states that jump can be canceled into"))
	TArray<FGameplayTag> cancelableJumpStates;

	virtual void BeginPlay() override;

public:
	UPlayerCombatCancelComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	bool CanCancel(FGameplayTag& DesiredStateTag) const;
	void SetCanCancelCurrentAction(bool bCanCancel);
};
