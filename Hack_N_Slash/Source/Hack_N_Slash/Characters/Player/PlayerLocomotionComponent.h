// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../../Interfaces/LocomotionCmdInterface.h"
#include "PlayerLocomotionComponent.generated.h"

/**
 * Player locomotion driver (Option B)
 * Implements locomotion interface so states can command movement without touching ACharacter directly.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HACK_N_SLASH_API UPlayerLocomotionComponent : public UActorComponent, public ILocomotionCmdInterface
{
    GENERATED_BODY()

private:
	UPROPERTY()
    class ACharacter* OwnerChar {nullptr};

    bool EnsureOwnerCharacter();

protected:
    UPROPERTY(EditAnywhere)
    bool bDebug {false};
    
    virtual void BeginPlay() override;

public:
	UPlayerLocomotionComponent();

    // ILocomotionCommandInterface
    virtual void AddMoveInputScaled(const FVector2D& Move, float Scale) override;
    virtual void AddLookInputScaled(const FVector2D& Look, float YawRate, float PitchRate) override;

    virtual void JumpPressed() override;
    virtual void JumpReleased() override;
    virtual void LaunchUp(float JumpZ) override;
};