// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../../Interfaces/CombatCmdInterface.h"
#include "../../Enums/ECombatVulnerability.h"
#include "EnemyCombatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSuperArmorActivated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSuperArmorDeactivated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSuperArmorBroken);

class ICombatInstigator;
class UBaseCharAnimInstance;
class UCombatResolutionComponent;
class UCombatTraceComponent;
class UEnemyBrainComponent;
class UStateMachineComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HACK_N_SLASH_API UEnemyCombatComponent : public UActorComponent, public ICombatCmdInterface
{
	GENERATED_BODY()

private:
	UPROPERTY(Transient) UBaseCharAnimInstance* animInst = nullptr;
	UPROPERTY(Transient) ACharacter* ownerChar = nullptr;
	UPROPERTY(Transient) UStateMachineComponent* stateMachineComp = nullptr;
	UPROPERTY(Transient) UCombatResolutionComponent* combatResComp = nullptr;
	UPROPERTY(Transient) UCombatTraceComponent* traceComp = nullptr;
	UPROPERTY(Transient) UEnemyBrainComponent* enemyBrainComp = nullptr;

	bool EnsureReferences();
	UFUNCTION() void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

protected:
	UPROPERTY(EditAnywhere, Category = "Combat")
	bool bDebug = false;

    UPROPERTY(VisibleAnywhere, Category = "Combat")
    bool bHasSuperArmor = false;
	
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UPROPERTY(BlueprintAssignable)
	FOnSuperArmorActivated OnSuperArmorActivated;

	UPROPERTY(BlueprintAssignable)
	FOnSuperArmorDeactivated OnSuperArmorDeactivated;

	UPROPERTY(BlueprintAssignable)
	FOnSuperArmorBroken OnSuperArmorBroken;

	UEnemyCombatComponent();

    UFUNCTION(BlueprintNativeEvent, Category = "Combat")
	void ReceieveHit(UPARAM(ref) FAtkHitData& HitData);
	virtual void ReceieveHit_Implementation(FAtkHitData& HitData);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ActivateSuperArmor();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void DeactivateSuperArmor();
    
    bool HasSuperArmor() const { return bHasSuperArmor; }

	/* Combat Command Interface Functions*/
	virtual void AttackIntent(const FEnemyAtkData& AtkData) override;
	virtual void BlockStartIntent() override;
	virtual void BlockStopIntent() override;
};