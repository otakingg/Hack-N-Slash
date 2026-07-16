#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"

#include "../../Interfaces/CombatInstigator.h"
#include "../../Interfaces/Damageable.h"
#include "Player_Base.generated.h"

class UCharacterMovementComponent;
class UCombatResolutionComponent;
class UCombatTraceComponent;
class ULocomotionComponent;
class UPlayerCamComponent;
class UPlayerCombatComponent;
class UPlayerInputComponent;
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

    UPROPERTY(VisibleAnywhere, Category = "Player|Tags")
    FGameplayTagContainer gameplayTags;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Tags", meta = (Categories = "State.Movement."))
    FGameplayTag airborneTag;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Tags", meta = (Categories = "State.Movement."))
    FGameplayTag groundedTag;

	UPROPERTY(Transient) UCharacterMovementComponent* moveComp = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCombatResolutionComponent* combatResComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCombatTraceComponent* combatTraceComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ULocomotionComponent* locoComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UPlayerCamComponent* playerCamComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UPlayerCombatComponent* combatComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UPlayerInputComponent * inputComp;

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
	FOnTagsUpdated OnTagsUpdated;

	UPROPERTY(BlueprintAssignable)
	FOnHit OnHit;

	APlayer_Base();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void TryAction(const FGameplayTag& Action, const FVector2D& Look, const FVector2D& Move);
	void TryBufferedAction(const FGameplayTag& Action, const FVector2D& Move);

	void HandleActorDeath(AActor* Actor);

	/* Combat Instigator Interface Functions*/
	virtual const FGameplayTagContainer& GetTags() const override;

	UFUNCTION(BlueprintCallable, Category = "Player") virtual void AddTag(const FGameplayTag& Tag) override;
	UFUNCTION(BlueprintCallable, Category = "Player") virtual void RemoveTag(const FGameplayTag& Tag) override;
	UFUNCTION(BlueprintPure, Category = "Player") virtual bool HasTag(const FGameplayTag& Tag, bool bExact = false) const override;
	UFUNCTION(BlueprintPure, Category = "Player") virtual bool HasAnyTag(const TArray<FGameplayTag>& TagArray, bool bExact = false) const override;
	UFUNCTION(BlueprintPure, Category = "Player") virtual bool HasAllTags(const TArray<FGameplayTag>& TagArray, bool bExact = false) const override;
	UFUNCTION(BlueprintPure, Category = "Player") virtual bool IsAirborne() const override;
	UFUNCTION(BlueprintPure, Category = "Player") virtual bool IsGrounded() const override;

	virtual AActor* GetCurrentTarget() const override;
	virtual bool GetLockedOn() const override;
	
	/* Damageable Interface Functions*/
	virtual void Countered(AActor* Counteror, const FString& Reason) override;
	virtual bool IsAlive() const override;
	virtual void ReceiveHit(FAtkHitData& HitData) override;
};
