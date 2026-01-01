// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "States/CharacterState.h"
#include "StateMachineComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HACK_N_SLASH_API UStateMachineComponent : public UActorComponent
{
	GENERATED_BODY()

private:
    UPROPERTY()
    UCharacterState* currentState;

    UPROPERTY()
    UCharacterState* previousState {nullptr};
    
protected:
    //One persistent instance per state class
    //In the Editor populate the map with: IdleState → nullptr, LightAttackState → nullptr, HitReactState → nullptr, etc.
    //The component creates the instances at runtime
    UPROPERTY(EditDefaultsOnly, Instanced)
    TMap<TSubclassOf<UCharacterState>, UCharacterState*> stateInstances;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	UStateMachineComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /** State Control */
    //How to request a state change cleanly: ChangeState(stateInstances[UHitReactState::StaticClass()], true);
    void ChangeState(UCharacterState*, bool);

    /** Queries */
    UCharacterState* GetCurrentState() const;
    UCharacterState* GetPreviousState() const;

    FGameplayTag GetCurrentStateTag() const;
    bool IsInStateTag(FGameplayTag) const; //Tag-based query (decoupled & hierarchy-friendly)
};
