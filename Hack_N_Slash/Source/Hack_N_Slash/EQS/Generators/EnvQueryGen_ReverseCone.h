// Fill out your copyright notice in the Description page of Project Settings
// Helpful video: https://youtu.be/GVsZqBWldQ8?si=KKvf8UNWa4H-Cl3a

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/Generators/EnvQueryGenerator_Cone.h"
#include "EnvQueryGen_ReverseCone.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "Points: Reverse Cone"))
class HACK_N_SLASH_API UEnvQueryGen_ReverseCone : public UEnvQueryGenerator_Cone
{
	GENERATED_BODY()

public:
	virtual void GenerateItems(FEnvQueryInstance& QueryInstance) const override;

	//Returns the title of the generator on the corresponding node in the EQS Editor window
	virtual FText GetDescriptionTitle() const override;
};