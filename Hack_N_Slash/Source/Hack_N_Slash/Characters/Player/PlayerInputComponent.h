#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "../../Enums/EStickMovement.h"
#include "PlayerInputComponent.generated.h"

// This class handles player input registration logic (Although the state machine handles interpreting player input)
// Registers inputs (Is is an attack? jump? look? etc.)
// Hold functionality for adding actions resolved by the state machine into a buffer if they couldn't be done immediately
// Stores movement inputs for motion detection (Like detecting a circle motion)

class APlayer_Base;
class ICombatInstigator;
class UStateMachineComponent;

USTRUCT(BlueprintType)
struct FBufferedAction // The current buffered action
{
    GENERATED_BODY()

	UPROPERTY(VisibleAnywhere) float time = -1.0f; // The time this action was entered into the buffer
    UPROPERTY(VisibleAnywhere) FGameplayTag action; // The action being buffered
	UPROPERTY(VisibleAnywhere) FVector2D move = FVector2D::ZeroVector; // The movement input vector associated with the action
};

USTRUCT(BlueprintType)
struct FMoveInput // Holds information about a movement input
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere) float time = -1.0f;
	UPROPERTY(VisibleAnywhere) EStickDirection direction = EStickDirection::Any;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HACK_N_SLASH_API UPlayerInputComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	APlayer_Base* player = nullptr;
	UStateMachineComponent* stateMachineComp = nullptr;
	ICombatInstigator* iCmbtInst = nullptr;

	//FTimerHandle TH_AttackHeavy;
	//FTimerHandle TH_AttackLight;

	static int32 DirectionToIndex(EStickDirection Direction); // Maps each of the 8 cardinal input directions to an integer

	bool AreDirectionsAdjacent(EStickDirection DirectionA, EStickDirection DirectionB, int32 Tolerance) const; // Determinces if 2 directions are adjacent. "Tolerance" determines what adjacent means
	bool PerformedCircle() const;
	bool PerformedLinearMotion(EStickDirection Start, EStickDirection End) const; // BackForward, LeftRight, etc.

	//UFUNCTION() void HandlePlayerInputHelper(EPlayerInput PlayerInput, const FVector2D LookVector, const FVector2D MoveVector);

protected:
	//UPROPERTY(EditAnywhere, Category = "Input", meta = (Tooltip = "The time after recieving an input for it to be registered by the system"))
	//float inputRegisterTime = 0.05f;

	UPROPERTY(EditAnywhere, Category = "Input|Buffer", meta = (ToolTip = "Max amount of time before a buffered input is forgotten"))
	float actionBufferMaxTime = 0.25f;

	UPROPERTY(VisibleAnywhere, Category = "Input|Buffer", meta = (ToolTip = "Buffered input information"))
	FBufferedAction bufferedAction;

	UPROPERTY(EditAnywhere, Category = "Input|History", meta = (ToolTip = "Max amount of time before a move input is forgotten"))
	float moveInputHistoryMaxTime = 0.35f;

	UPROPERTY(VisibleAnywhere, Category = "Input|History", meta = (ToolTip = "The last 16 directions that the move input made"))
	TArray<FMoveInput> moveInputHistory; // This is how we can detect complex motions like circles

	UPROPERTY(VisibleAnywhere, Category = "Input|Heavy", meta = (ToolTip = "How long this input has been held"))
	float heldTimeAtkHeavy = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Input|Light", meta = (ToolTip = "How long this input has been held"))
	float heldTimeAtkLight = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Input|Heavy", meta = (ToolTip = "When was the input started"))
	float startTimeAtkHeavy = -1.0f;

	UPROPERTY(VisibleAnywhere, Category = "Input|Light", meta = (ToolTip = "When was the input started"))
	float startTimeAtkLight = -1.0f;


	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "Player Input")
	void HandlePlayerInput(EPlayerInput PlayerInput, const FVector2D LookVector = FVector2D::ZeroVector, const FVector2D MoveVector = FVector2D::ZeroVector);

public:
	UPlayerInputComponent();

	FVector GetInputWorldDirRelativeToCamOrTarget(const FVector2D& InputVector, FVector& OutLocalForward, FVector& OutLocalRight, AActor* Target = nullptr) const;
    EStickDirection GetStickDirFromWorldDir(const FVector& WorldDir, const FVector& LocalForward, const FVector& LocalRight) const;
    EStickDirection GetWorldDirRelativeToPlayerFacing(const FVector& WorldDir) const;

	/* --------------- Input Timing ---------------------------*/
	float GetHeldTimeAtkHeavy() const { return heldTimeAtkHeavy; }
	float GetHeldTimeAtkLight() const { return heldTimeAtkLight; }

	void ResetInputTimings();

	/* --------------- Buffer ---------------------------*/
	void SetActionBuffer(const FGameplayTag& Action, const FVector2D& Move = FVector2D::ZeroVector);
	void ClearActionBuffer();

	/* --------------- Move Input History ---------------------------*/
	void AddToMoveInputHistory(const FVector2D& Move);
	bool PerformedDirection(EStickDirection Direction, const FVector2D& Move) const;
	bool PerformedMotion(EStickMotion Motion);
};