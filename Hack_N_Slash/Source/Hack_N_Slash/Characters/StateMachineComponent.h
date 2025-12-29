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
    TMap<TSubclassOf<UCharacterState>, UCharacterState*> stateInstances;

    void ChangeStateInternal(UCharacterState*, bool);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	UStateMachineComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /** State Control */
    bool TryChangeState(TSubclassOf<UCharacterState>);
    void ForceChangeState(TSubclassOf<UCharacterState>);

    /** Queries */
    UCharacterState* GetCurrentState() const;
    bool IsInStateTag(FGameplayTag) const;
};
