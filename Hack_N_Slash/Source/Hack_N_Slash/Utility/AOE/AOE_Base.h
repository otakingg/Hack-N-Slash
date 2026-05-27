// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AOE_Base.generated.h"

UCLASS()
class HACK_N_SLASH_API AAOE_Base : public AActor
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	AAOE_Base();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "AOE")
	void OnAOEOverlap(AActor* actor);
	virtual void OnAOEOverlap_Implementation(AActor* actor) {}
};