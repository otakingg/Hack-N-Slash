#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"

#include "../../Interfaces/CombatInstigator.h"
#include "../../Interfaces/Damageable.h"
#include "../../Interfaces/Enemy.h"
#include "EnemyBase.generated.h"

class APlayer_Base;
class UCapsuleComponent;
class UCharacterMovementComponent;
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

    UPROPERTY(VisibleAnywhere, Category = "Enemy|Tags")
    FGameplayTagContainer gameplayTags;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Tags", meta = (Categories = "State.Movement."))
    FGameplayTag airborneTag;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Tags", meta = (Categories = "State.Movement."))
    FGameplayTag groundedTag;

	UPROPERTY(Transient) UCharacterMovementComponent* moveComp = nullptr;
	
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
	FOnTagsUpdated OnTagsUpdated;

	UPROPERTY(BlueprintAssignable)
	FOnHit OnHit;

	AEnemyBase();
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override; // Called to bind functionality to input

	/* Combat Instigator Interface Functions*/
	virtual const FGameplayTagContainer& GetTags() const override;
	UFUNCTION(BlueprintCallable, Category = "Enemy") virtual void AddTag(const FGameplayTag& Tag) override;
	UFUNCTION(BlueprintCallable, Category = "Enemy") virtual void RemoveTag(const FGameplayTag& Tag) override;
	UFUNCTION(BlueprintPure, Category = "Enemy") virtual bool HasTag(const FGameplayTag& Tag, bool bExact = false) const override;
	UFUNCTION(BlueprintPure, Category = "Enemy") virtual bool HasAnyTag(const TArray<FGameplayTag>& TagArray, bool bExact = false) const override;
	UFUNCTION(BlueprintPure, Category = "Enemy") virtual bool HasAllTags(const TArray<FGameplayTag>& TagArray, bool bExact = false) const override;
	UFUNCTION(BlueprintPure, Category = "Enemy") virtual bool IsAirborne() const override;
	UFUNCTION(BlueprintPure, Category = "Enemy") virtual bool IsGrounded() const override;
	
	virtual AActor* GetCurrentTarget() const override;
	virtual bool GetLockedOn() const override;
	
	/* Damageable Interface Functions*/
	virtual void Countered(AActor* Counteror, const FString& Reason) override;
	virtual bool IsAlive() const override;
	virtual void ReceiveHit(FAtkHitData& HitData) override;
};
