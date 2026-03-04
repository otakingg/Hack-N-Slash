// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../../Interfaces/CombatInstigator.h"
#include "../../Interfaces/Damageable.h"
#include "../../Interfaces/PlayerInt.h"
#include "../../Interfaces/Targetable.h"
#include "Player_Base.generated.h"

UCLASS()
class HACK_N_SLASH_API APlayer_Base : public ACharacter, public IPlayerInt, public ICombatInstigator, public IDamageable, public ITargetable
{
	GENERATED_BODY()

private:
	UPROPERTY() class UCameraComponent* camComp;
	UPROPERTY() class UCharacterMovementComponent* moveComp;
	UPROPERTY() class USpringArmComponent* springArmComp;

protected:
	UPROPERTY(EditAnywhere)
	bool bDebug {false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UPlayerLocomotionComponent* playerLocoComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UStateMachineComponent* stateMachineComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UStatsComponent* statsComp;
	
	virtual void BeginPlay() override;

public:
	APlayer_Base();
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

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
};
