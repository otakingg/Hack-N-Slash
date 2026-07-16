#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "PlayerInputComponent.generated.h"

class APlayer_Base;
class UStateMachineComponent;

USTRUCT(BlueprintType)
struct FBufferedAction
{
    GENERATED_BODY()

	UPROPERTY(VisibleAnywhere) float time = -1.0f;
    UPROPERTY(VisibleAnywhere) FGameplayTag action;
	UPROPERTY(VisibleAnywhere) FVector2D move = FVector2D::ZeroVector;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HACK_N_SLASH_API UPlayerInputComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	APlayer_Base* player = nullptr;
	UStateMachineComponent* stateMachineComp = nullptr;

protected:
	//UPROPERTY(EditAnywhere, Category = "Input", meta = (Tooltip = "The time after recieving an input for it to be registered"))
	//float inputRegisterTime = 0.1f;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (Tooltip = "How long a button has to be held before considered being held by the system"))
	float inputHeldThreshold = 0.1f;

	UPROPERTY(EditAnywhere, Category = "Input|Buffer", meta = (ToolTip = "Max amount of time before a buffered input is forgotten"))
	float actionBufferMaxTime = 0.25f;

	UPROPERTY(VisibleAnywhere, Category = "Input|Buffer", meta = (ToolTip = "Buffered input information"))
	FBufferedAction bufferedAction;

	UPROPERTY(VisibleAnywhere, Category = "Input|Heavy", meta = (ToolTip = "When was the input started"))
	float heavyStartTime = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Input|Light", meta = (ToolTip = "When was the input started"))
	float lightStartTime = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Input|Heavy", meta = (ToolTip = "Is heavy input held"))
	bool bHeavyHeld = false;

	UPROPERTY(VisibleAnywhere, Category = "Input|Light", meta = (ToolTip = "Is heavy input held"))
	bool bLightHeld = false;

	UPROPERTY(VisibleAnywhere, Category = "Input|Heavy", meta = (ToolTip = "How long this input has been held"))
	float heldTimeAtkHeavy = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Input|Light", meta = (ToolTip = "How long this input has been held"))
	float heldTimeAtkLight = 0.0f;


	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Player Input")
	void HandlePlayerInput(EPlayerInput PlayerInput, const FVector2D LookVector = FVector2D::ZeroVector, const FVector2D MoveVector = FVector2D::ZeroVector);

public:
	UPlayerInputComponent();

	void SetActionBuffer(const FGameplayTag& Action, const FVector2D& Move = FVector2D::ZeroVector);
	void ClearActionBuffer();
};
