#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "PlayerCombatCancelComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HACK_N_SLASH_API UPlayerCombatCancelComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Cancel")
	bool bDebug = false;

	UPROPERTY(VisibleAnywhere, Category = "Cancel")
	bool bCanCancelCurrentAction = false; // Will be set by outside sources (AnimNotifies, StateMachine, etc) to determine whether the current action can be canceled or not

	//virtual void BeginPlay() override;
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
public:
	UPlayerCombatCancelComponent();

	UFUNCTION(BlueprintPure, Category = "Player Combat Cancel")
	bool CanCancel(const FGameplayTag& CurrentStateTag, const TArray<FGameplayTag>& AllowedStates) const;
	void SetCanCancelCurrentAction(bool bCanCancel);
};