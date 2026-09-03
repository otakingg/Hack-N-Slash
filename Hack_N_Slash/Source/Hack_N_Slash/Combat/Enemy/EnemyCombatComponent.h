#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyCombatComponent.generated.h"

// This class handles all the enemy combat functionality
// Attacking, Blocking, etc.

class ICombatInstigator;
class UBaseCharAnimInstance;
class UCombatResolutionComponent;
class UCombatTraceComponent;
class ULocomotionComponent;
class UStateMachineComponent;
struct FAtkHitData;
struct FEnemyAtkData;


// Event Dispatchers for Super Armor functionality
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSuperArmorActivated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSuperArmorDeactivated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSuperArmorBroken);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HACK_N_SLASH_API UEnemyCombatComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	ICombatInstigator* iCmbtInst = nullptr;
	UPROPERTY(Transient) UBaseCharAnimInstance* animInst = nullptr;
	UPROPERTY(Transient) ACharacter* ownerChar = nullptr;
	UPROPERTY(Transient) UStateMachineComponent* stateMachineComp = nullptr;
	UPROPERTY(Transient) UCombatResolutionComponent* combatResComp = nullptr;
	UPROPERTY(Transient) UCombatTraceComponent* traceComp = nullptr;
	UPROPERTY(Transient) ULocomotionComponent* locoComp = nullptr;
	FTimerHandle TH_Vulnerable; // Timer handle for ending the vulnerability window

	bool EnsureReferences();
	UFUNCTION() void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted); // Handles functionality for when an attack finishes or gets interrupted

protected:
	UPROPERTY(EditAnywhere, Category = "Combat")
	bool bDebug = false;

    UPROPERTY(VisibleAnywhere, Category = "Combat")
    bool bHasSuperArmor = false;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ToolTip = "How long this enemy will stay vulnerable when becoming vulnerable"))
	float vulnerableDuration = 5.0f;

	virtual void BeginPlay() override;
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    //--------------------------------
    // Vulnerability
    //--------------------------------

    void EnterVulnerable(); // Sets poise to 0
    UFUNCTION() void ExitVulnerable(); // Returns poise back to what it was

public:
	UPROPERTY(BlueprintAssignable)
	FOnSuperArmorActivated OnSuperArmorActivated;

	UPROPERTY(BlueprintAssignable)
	FOnSuperArmorDeactivated OnSuperArmorDeactivated;

	UPROPERTY(BlueprintAssignable)
	FOnSuperArmorBroken OnSuperArmorBroken;

	UEnemyCombatComponent();

	/* -------------------- Super Armor -----------------------*/
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ActivateSuperArmor();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void DeactivateSuperArmor();
    
    bool HasSuperArmor() const { return bHasSuperArmor; }

	/* -------------------- Event Handling -----------------------*/
	void ReceieveHit(FAtkHitData& HitData); // Handles addtional functionality the enemy wants when processing a hit

	/* ----------------- Intents ---------------*/
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool Attack(const FEnemyAtkData& AtkData);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool BlockStart();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void BlockStop();
};