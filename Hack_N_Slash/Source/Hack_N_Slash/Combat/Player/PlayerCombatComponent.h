// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../../Interfaces/CombatCmdInterface.h"
#include "../../Structs/FPlayerAtkData.h"
#include "PlayerCombatComponent.generated.h"

class ICharAnimInterface;
class UCharacterMovementComponent;
class UCombatTraceComponent;
class UPlayerTargettingComponent;
class UStateMachineComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HACK_N_SLASH_API UPlayerCombatComponent : public UActorComponent, public ICombatCmdInterface
{
	GENERATED_BODY()

private:
	ICharAnimInterface* iCharAnimInst = nullptr;
	UPROPERTY() ACharacter* ownerChar = nullptr;
	UPROPERTY() UCharacterMovementComponent* moveComp = nullptr;
	UPROPERTY() UPlayerTargettingComponent* playerTargettingComp = nullptr;
	UPROPERTY() UStateMachineComponent* stateMachineComp = nullptr;
	UPROPERTY() UCombatTraceComponent* traceComp = nullptr;
	FPlayerAtkData* currentAtkData = nullptr;

	FTimerHandle TH_Dodge;

	bool EnsureReferences();

	FVector GetInputWorldDirRelativeToCamOrTarget(const FVector2D& InputVector, FVector& OutLocalForward, FVector& OutLocalRight, AActor* Target = nullptr) const;
    EStickMotion GetStickMotionFromWorldDir(const FVector& WorldDir, const FVector& LocalForward, const FVector& LocalRight) const;
    EStickMotion GetWorldDirRelativeToPlayerFacing(const FVector& WorldDir) const;

    bool IsAtkContextValid(const FPlayerAtkData &AtkData, EPlayerAction PlayerAction, const FVector2D &InputVector) const;
    void SnapToInputDirection(const FVector2D& InputDir);
    void PerformAttack(FPlayerAtkData* AtkData, const FVector2D& Dir);
	UFUNCTION() void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	UFUNCTION() void EndDodge(EStickMotion DodgeMotion);

protected:
	UPROPERTY(EditAnywhere, Category = "Combat")
	bool bDebug = false;
	
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	UDataTable* activeAtkDT = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge")
	UAnimMontage* airDodgeMont = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge")
	UAnimMontage* groundDodgeMontBack = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge")
	UAnimMontage* groundDodgeMontFwd = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge")
	UAnimMontage* groundDodgeMontLeft = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge")
	UAnimMontage* groundDodgeMontRight = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge")
	float dodgeDistance = 600.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge")
	float dodgeDuration = 0.2f;

	virtual void BeginPlay() override;

public:
	UPlayerCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void AttackHeavyStart(const FVector2D& InputVector);
	void AttackLightStart(const FVector2D& InputVector);
	//void BlockStart();
	//void BlockStop();

	void ClearAtkData();
	FPlayerAtkData* GetCurrentAtkData() const;

	/* Combat Command Interface Functions*/
	virtual void AttackIntent(const FVector2D& Dir, EPlayerAction PlayerAction) override;
	virtual void DodgeIntent(const FVector2D& Dir = FVector2D::ZeroVector) override;
};
