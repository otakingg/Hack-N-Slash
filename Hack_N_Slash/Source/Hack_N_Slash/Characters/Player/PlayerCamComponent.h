// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerCamComponent.generated.h"

class ICombatInstigator;
class IPlayerInt;
class UCameraComponent;
class UCharacterMovementComponent;
class USpringArmComponent;
class UStateMachineComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HACK_N_SLASH_API UPlayerCamComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY() UCameraComponent* camComp = nullptr;
	UPROPERTY() ACharacter* owner = nullptr;
	UPROPERTY() AController* controller = nullptr;
	UPROPERTY() UCharacterMovementComponent* moveComp = nullptr;
	UPROPERTY() USpringArmComponent* springArmComp = nullptr;
	UPROPERTY() UStateMachineComponent* stateMachineComp = nullptr;
	ICombatInstigator* iCmbtInst = nullptr;
	IPlayerInt* iPlayer = nullptr;

	float normalSpringLength = 300.0f;
	FVector normalSpringSocketOffset = FVector::ZeroVector;

	bool EnsureReferences();
	void UpdateLockOnCam(float DeltaTime);
	void UpdateLockOffCam(float DeltaTime);

protected:
	UPROPERTY(EditAnywhere, Category = "Camera|Look Input", meta = (ClampMin = "0.0"))
	float lookUpRate = 45.0f;

	UPROPERTY(EditAnywhere, Category = "Camera|Look Input", meta = (ClampMin = "0.0"))
	float turnRate = 45.0f;

	UPROPERTY(EditAnywhere, Category = "Camera|Lag")
	bool bCameraLag = true;

	UPROPERTY(EditAnywhere, Category = "Camera|Lag", meta = (ClampMin = "0.0"))
	float speedCamLag = 10.0f;

	UPROPERTY(EditAnywhere, Category = "Camera|Transition", meta = (ToolTip = "Rotation"), meta = (ClampMin = "0.1"))
	float speedRot = 90.0f;
	
	UPROPERTY(EditAnywhere, Category = "Camera|Transition", meta = (ToolTip = "Translation"), meta = (ClampMin = "0.1"))
	float speedTrans = 400.0f;

	UPROPERTY(EditAnywhere, Category = "Camera|Transition", meta = (ClampMin = "0.1"))
	float speedZoom = 80.0f;

	UPROPERTY(EditAnywhere, Category = "Camera|Lock On", meta = (ToolTip = "0.0 = pure enemy, 1.0 = pure midpoint"))
	float midPointBias = 0.25f;  

	UPROPERTY(EditAnywhere, Category = "Camera|Lock On")
	FVector airSpringOffsetLockOn = {0.f, 80.f, 80.f};

	UPROPERTY(EditAnywhere, Category = "Camera|Lock On")
	FVector groundSpringOffsetLockOn = {0.f, 50.f, 30.f};

    UPROPERTY(EditAnywhere, Category = "Camera|Lock On")
    float airSpringPitchOffsetLockOn = 15.0f;

    UPROPERTY(EditAnywhere, Category = "Camera|Lock On")
    float groundSpringPitchOffsetLockOn = 5.0f;

    UPROPERTY(EditAnywhere, Category = "Camera|Lock On", meta = (ClampMin = "0.0"))
    float springZoomMinLockOn = 400.f;

    UPROPERTY(EditAnywhere, Category = "Camera|Lock On", meta = (ClampMin = "0.0"))
    float springZoomMaxLockOn = 650.f;

	virtual void BeginPlay() override;

public:
	UPlayerCamComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void AddLookInputScaled(const FVector2D& Look);
};
