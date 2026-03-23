// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerCamComponent.generated.h"

class UCameraComponent;
class USpringArmComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HACK_N_SLASH_API UPlayerCamComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY() UCameraComponent* camComp = nullptr;
	UPROPERTY() ACharacter* owner = nullptr;
	UPROPERTY() USpringArmComponent* springArmComp = nullptr;

protected:
	UPROPERTY(EditAnywhere)
	bool bDebug = false;

	UPROPERTY(EditAnywhere, Category = Camera, meta = (ClampMin = "0.0"))
	float lookUpRate = 45.0f;

	UPROPERTY(EditAnywhere, Category = Camera, meta = (ClampMin = "0.0"))
	float turnRate = 45.0f;

	virtual void BeginPlay() override;

public:
	UPlayerCamComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void AddLookInputScaled(const FVector2D& Look);
};
