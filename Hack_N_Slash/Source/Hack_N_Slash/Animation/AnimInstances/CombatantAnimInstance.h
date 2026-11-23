#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameplayTagContainer.h"
#include "CombatantAnimInstance.generated.h"

class ICombatant;

/**
 * 
 */
UCLASS()
class HACK_N_SLASH_API UCombatantAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
protected:
	ICombatant* iCombatant = nullptr;

    UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Combatant")
    APawn* ownerPawn = nullptr;

    UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Combatant")
    FGameplayTagContainer combatantTags;

    UFUNCTION(BlueprintCallable, Category = "Anim|Combatant")
    void InitializeAnimation();

    UFUNCTION(BlueprintCallable, Category = "Anim|Combatant")
    void UpdateAnimation(float DeltaSeconds);

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|Combatant")
    bool bDebug = false;
    
    UFUNCTION(BlueprintCallable, Category = "Anim|Combatant")
    float PlayMontage(UAnimMontage* Montage = nullptr, FName Section = NAME_None);
};