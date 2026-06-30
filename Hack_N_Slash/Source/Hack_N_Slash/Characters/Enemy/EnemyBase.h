// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../../Interfaces/CombatInstigator.h"
#include "../../Interfaces/Damageable.h"
#include "../../Interfaces/Enemy.h"
#include "EnemyBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyHit, const FAtkHitData&, HitData);

class APlayer_Base;
class UCapsuleComponent;
class UCombatResolutionComponent;
class UCombatTraceComponent;
class UEnemyBrainComponent;
class UEnemyCombatComponent;
class ULocomotionComponent;
class UStateMachineComponent;
class UStatsComponent;

UCLASS()
class HACK_N_SLASH_API AEnemyBase : public ACharacter, public ICombatInstigator, public IDamageable, public IEnemy
{
	GENERATED_BODY()

private:
	UPROPERTY(Transient)
	APlayer_Base* player = nullptr;

protected:
	UPROPERTY(EditAnywhere, Category = "Enemy")
	bool bDebug = false;

    UPROPERTY(VisibleAnywhere, Category = "Player|Tags")
    FGameplayTagContainer overrideTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UEnemyBrainComponent* brainComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UEnemyCombatComponent* combatComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCombatResolutionComponent* combatResComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCombatTraceComponent* combatTraceComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ULocomotionComponent* locoComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStateMachineComponent* stateMachineComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStatsComponent* statsComp;

	virtual void BeginPlay() override; // Called when the game starts or when spawned
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(BlueprintAssignable)
	FOnEnemyHit OnHit;

	AEnemyBase();
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override; // Called to bind functionality to input

	/* Combat Instigator Interface Functions*/
	virtual AActor* GetCurrentTarget() const override;
	virtual bool GetLockedOn() const override;

    UFUNCTION(BlueprintPure, Category = "Tags")
    virtual bool HasOverrideExact(FGameplayTag& Tag) const override { return Tag.IsValid() && overrideTags.HasTagExact(Tag); }

    UFUNCTION(BlueprintCallable, Category = "Tags")
    virtual void AddOverrideTag(const FGameplayTag& OverrideTag) override;
    
    UFUNCTION(BlueprintCallable, Category = "Tags")
    virtual void RemoveOverrideTag(const FGameplayTag& OverrideTag) override;
	
	/* Damageable Interface Functions*/
	virtual void AttackDetected(AActor* Attacker) override;
	virtual void Countered(AActor* Counteror, const FString& Reason) override;
	virtual bool IsAlive() const override;
	virtual void ReceiveHit(FAtkHitData& HitData) override;
};
