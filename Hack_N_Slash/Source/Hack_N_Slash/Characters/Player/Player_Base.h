// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../../Interfaces/CombatInstigator.h"
#include "../../Interfaces/Damageable.h"
#include "Player_Base.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerHit, const FAtkHitData&, HitData);

class UCombatResolutionComponent;
class UCombatTraceComponent;
class ULocomotionComponent;
class UPlayerCamComponent;
class UPlayerCombatCancelComponent;
class UPlayerCombatComponent;
class UPlayerTargettingComponent;
class UStateMachineComponent;
class UStatsComponent;

UCLASS()
class HACK_N_SLASH_API APlayer_Base : public ACharacter, public ICombatInstigator, public IDamageable
{
	GENERATED_BODY()

private:
	FTimerHandle TH_Input_Move;

protected:
	UPROPERTY(EditAnywhere, Category = "Player")
	bool bDebug = false;

	UPROPERTY(EditAnywhere, Category = "Player", meta = (Tooltip = "The time after recieving an input that the system will wait before executing logic"))
	float inputRegisterTime = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCombatResolutionComponent* combatResComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCombatTraceComponent* combatTraceComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ULocomotionComponent* locoComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UPlayerCamComponent* playerCamComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UPlayerCombatCancelComponent* playerCombatCancelComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UPlayerCombatComponent* combatComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UPlayerTargettingComponent* playerTargettingComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStateMachineComponent* stateMachineComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStatsComponent* statsComp;
	
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UPROPERTY(BlueprintAssignable)
	FOnPlayerHit OnHit;

	APlayer_Base();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Inputs */
	/*UFUNCTION(BlueprintCallable, Category = "Player Input") void Input_FaceEast_Started(const FVector2D& InputVector);
	UFUNCTION(BlueprintCallable, Category = "Player Input") void Input_FaceEast_OnGoing(const FVector2D& InputVector);
	UFUNCTION(BlueprintCallable, Category = "Player Input") void Input_FaceEast_Completed(const FVector2D& InputVector);*/

	UFUNCTION(BlueprintCallable, Category = "Player Input") void Input_FaceNorth_Started(const FVector2D& InputVector);
	UFUNCTION(BlueprintCallable, Category = "Player Input") void Input_FaceNorth_OnGoing(const FVector2D& InputVector);
	UFUNCTION(BlueprintCallable, Category = "Player Input") void Input_FaceNorth_Completed(const FVector2D& InputVector);

	UFUNCTION(BlueprintCallable, Category = "Player Input") void Input_FaceSouth_Started(const FVector2D& InputVector);
	UFUNCTION(BlueprintCallable, Category = "Player Input") void Input_FaceSouth_OnGoing(const FVector2D& InputVector);
	UFUNCTION(BlueprintCallable, Category = "Player Input") void Input_FaceSouth_Completed(const FVector2D& InputVector);

	UFUNCTION(BlueprintCallable, Category = "Player Input") void Input_FaceWest_Started(const FVector2D& InputVector);
	UFUNCTION(BlueprintCallable, Category = "Player Input") void Input_FaceWest_OnGoing(const FVector2D& InputVector);
	UFUNCTION(BlueprintCallable, Category = "Player Input") void Input_FaceWest_Completed(const FVector2D& InputVector);

	UFUNCTION(BlueprintCallable, Category = "Player Input") void Input_Mouse_Triggered(const FVector2D& InputVector); // Move the mouse

	/*UFUNCTION(BlueprintCallable, Category = "Player Input") void Input_ShoulderLeft_Started(const FVector2D& InputVector);
	UFUNCTION(BlueprintCallable, Category = "Player Input") void Input_ShoulderLeft_OnGoing(const FVector2D& InputVector);
	UFUNCTION(BlueprintCallable, Category = "Player Input") void Input_ShoulderLeft_Completed(const FVector2D& InputVector);*/

	/*UFUNCTION(BlueprintCallable, Category = "Player Input") void Input_ShoulderRight_Started(const FVector2D& InputVector);
	UFUNCTION(BlueprintCallable, Category = "Player Input") void Input_ShoulderRight_OnGoing(const FVector2D& InputVector);
	UFUNCTION(BlueprintCallable, Category = "Player Input") void Input_ShoulderRight_Completed(const FVector2D& InputVector);*/

	//UFUNCTION(BlueprintCallable, Category = "Player Input") void Input_StickButtonLeft_Started(const FVector2D& InputVector); // Press the stick
	UFUNCTION(BlueprintCallable, Category = "Player Input") void Input_StickButtonRight_Started(const FVector2D& InputVector);

	UFUNCTION(BlueprintCallable, Category = "Player Input") void Input_StickLeft_Triggered(const FVector2D& InputVector); // Tilt the stick
	UFUNCTION(BlueprintCallable, Category = "Player Input") void Input_StickRight_Triggered(const FVector2D& InputVector);

	/*UFUNCTION(BlueprintCallable, Category = "Player Input") void Input_TriggerLeft_Started(const FVector2D& InputVector);
	UFUNCTION(BlueprintCallable, Category = "Player Input") void Input_TriggerLeft_OnGoing(const FVector2D& InputVector);
	UFUNCTION(BlueprintCallable, Category = "Player Input") void Input_TriggerLeft_Completed(const FVector2D& InputVector);*/

	UFUNCTION(BlueprintCallable, Category = "Player Input") void Input_TriggerRight_Started(const FVector2D& InputVector);
	UFUNCTION(BlueprintCallable, Category = "Player Input") void Input_TriggerRight_OnGoing(const FVector2D& InputVector);
	//UFUNCTION(BlueprintCallable, Category = "Player Input") void Input_TriggerRight_Completed(const FVector2D& InputVector);

	void HandleActorDeath(AActor* Actor);

	/* Combat Instigator Interface Functions*/
	virtual AActor* GetCurrentTarget() const override;
	virtual bool GetLockedOn() const override;
	
	/* Damageable Interface Functions*/
	virtual bool IsAlive() const override;
	virtual void ReceiveHit(FAtkHitData& HitData) override;
};
