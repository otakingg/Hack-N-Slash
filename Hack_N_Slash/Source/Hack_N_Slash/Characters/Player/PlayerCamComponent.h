#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerCamComponent.generated.h"

class UCameraComponent;
class UCharacterMovementComponent;
class UPlayerTargettingComponent;
class USpringArmComponent;
class UStateMachineComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HACK_N_SLASH_API UPlayerCamComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY() UCameraComponent* camComp = nullptr;
	UPROPERTY() ACharacter* owner = nullptr;
	UPROPERTY() AController* controller = nullptr;
	UPROPERTY() UCharacterMovementComponent* moveComp = nullptr;
	UPROPERTY() UPlayerTargettingComponent* playerTargettingComp = nullptr;
	UPROPERTY() USpringArmComponent* springArmComp = nullptr;
	UPROPERTY() UStateMachineComponent* stateMachineComp = nullptr;

	float normalSpringLength = 300.0f;
	FVector normalSpringSocketOffset = FVector::ZeroVector;

	bool EnsureReferences();

	void UpdateLockOnCam(float DeltaTime);
	void UpdateLockOffCam(float DeltaTime);

	FVector GetActorFocusPoint(const AActor* Actor, float HeightOffset) const;
	bool IsGrounded() const;

protected:
	UPROPERTY(EditAnywhere, Category="Camera|Look Input", meta=(ClampMin="0.0"))
	float lookUpRate = 45.0f;

	UPROPERTY(EditAnywhere, Category="Camera|Look Input", meta=(ClampMin="0.0"))
	float turnRate = 45.0f;

	UPROPERTY(EditAnywhere, Category="Camera|Transition", meta=(ClampMin="0.1"))
	float speedRot = 8.0f;

	UPROPERTY(EditAnywhere, Category="Camera|Transition", meta=(ClampMin="0.1"))
	float speedTrans = 6.0f;

	UPROPERTY(EditAnywhere, Category="Camera|Transition", meta=(ClampMin="0.1"))
	float speedZoom = 6.0f;

	UPROPERTY(EditAnywhere, Category="Camera|Lock On", meta=(ClampMin="0.0"))
	float playerFocusHeight = 60.0f;

	UPROPERTY(EditAnywhere, Category="Camera|Lock On", meta=(ClampMin="0.0"))
	float targetFocusHeight = 70.0f;

	UPROPERTY(EditAnywhere, Category="Camera|Lock On", meta=(ClampMin="0.0", ClampMax="1.0", ToolTip="0 = Pure player focus, 1 = Pure target focus"))
	float focusBiasToTarget = 0.65f;

	UPROPERTY(EditAnywhere, Category="Camera|Lock On")
	FVector groundSpringOffsetLockOn = FVector(0.0f, 55.0f, 35.0f);

	UPROPERTY(EditAnywhere, Category="Camera|Lock On")
	FVector airSpringOffsetLockOn = FVector(0.0f, 80.0f, 80.0f);

	UPROPERTY(EditAnywhere, Category="Camera|Lock On", meta=(ClampMin="0.0"))
	float springZoomMinLockOn = 400.0f;

	UPROPERTY(EditAnywhere, Category="Camera|Lock On", meta=(ClampMin="0.0"))
	float springZoomMaxLockOn = 650.0f;

	UPROPERTY(EditAnywhere, Category="Camera|Lock On", meta=(ClampMin="0.0"))
	float lockOnZoomScale = 0.6f;

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
public:
	UPlayerCamComponent();

	void AddLookMouseInput(const FVector2D& Look);
	void AddLookStickInput(const FVector2D& Look);
};