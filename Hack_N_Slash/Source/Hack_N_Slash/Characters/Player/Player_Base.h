// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../../Interfaces/CombatInstigator.h"
#include "../../Interfaces/Damageable.h"
#include "../../Interfaces/PlayerInt.h"
#include "../../Interfaces/Targetable.h"
#include "Player_Base.generated.h"

class ICharAnimInterface;
class UCameraComponent;
class UCombatResolutionComponent;
class UCharacterMovementComponent;
class UPlayerLocomotionComponent;
class USpringArmComponent;
class UStateMachineComponent;
class UStatsComponent;

UCLASS()
class HACK_N_SLASH_API APlayer_Base : public ACharacter, public IPlayerInt, public ICombatInstigator, public IDamageable, public ITargetable
{
	GENERATED_BODY()

private:
	UPROPERTY() UCameraComponent* camComp;
	UPROPERTY() UCharacterMovementComponent* moveComp;
	UPROPERTY() USpringArmComponent* springArmComp;

	ICharAnimInterface* iParentAnimInst;
	ICharAnimInterface* iChildAnimInst;

	void PlayAdditiveFlinch(FVector Direction);

protected:
	UPROPERTY(EditAnywhere)
	bool bDebug {false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCombatResolutionComponent* combatResComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UPlayerLocomotionComponent* playerLocoComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStateMachineComponent* stateMachineComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStatsComponent* statsComp;
	
	virtual void BeginPlay() override;

public:
	APlayer_Base();
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	ICharAnimInterface* GetParentAnimInterface() const { return iParentAnimInst; }
	ICharAnimInterface* GetChildAnimInterface() const { return iChildAnimInst; }

	/** Inputs */
	UFUNCTION(BlueprintCallable)
    void Input_AttackHeavy(const FVector2D& InputVector);

	UFUNCTION(BlueprintCallable)
    void Input_AttackLight(const FVector2D& InputVector);
	
	UFUNCTION(BlueprintCallable)
    void Input_BlockDodge(const FVector2D& InputVector);

	UFUNCTION(BlueprintCallable)
    void Input_BlockReleased();

	UFUNCTION(BlueprintCallable)
    void Input_JumpPressed();

	UFUNCTION(BlueprintCallable)
    void Input_JumpReleased();

	UFUNCTION(BlueprintCallable)
    void Input_Look(const FVector2D& InputVector);

	UFUNCTION(BlueprintCallable)
    void Input_Move(const FVector2D& InputVector);

	/* Damageable Interface Functions*/
	virtual void ReceiveHit(FAtkHitData& HitData) override;
};
