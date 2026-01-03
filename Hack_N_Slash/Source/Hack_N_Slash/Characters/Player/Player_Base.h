// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Player_Base.generated.h"

UCLASS()
class HACK_N_SLASH_API APlayer_Base : public ACharacter
{
	GENERATED_BODY()

private:
	class UCameraComponent* camComp;
	class UCharacterMovementComponent* movementComp;
	class USpringArmComponent* springArmComp;

protected:
	UPROPERTY(EditAnywhere)
	bool bDebug {false};
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UStateMachineComponent* stateMachineComp;

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
    void Input_Jump();

	UFUNCTION(BlueprintCallable)
    void Input_Look(const FVector2D& InputVector);

	UFUNCTION(BlueprintCallable)
    void Input_Move(const FVector2D& InputVector);
};
