// Fill out your copyright notice in the Description page of Project Settings.
#pragma once
#include "CoreMinimal.h"
#include "FAtkHitData.generated.h"

USTRUCT(BlueprintType)
struct FAtkHitData
{
    GENERATED_BODY()

    //Who
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<AActor> attacker {nullptr};

    //Where / How
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FVector hitLoc {FVector::ZeroVector};

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FVector hitDir {FVector::ZeroVector};

    //Raw attack values (pre-defense)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float baseDmgHP {0.f};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float baseDmgStagger {0.f};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float penetration {0.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ToolTip = "This will be multiplied by the strength stat of the character"))
	float multiplierDmgHP {1.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ToolTip = "This will be multiplied by the stagger strength stat of the character"))
	float multiplierDmgStagger {1.0f};

    //UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//bool bCanBeBlocked {true};

    //UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//bool bCanBeParried {true};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseKnockbackStopTime {false}; //Should the buffer movement stop after "X" amount of time

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector knockbackVelocity {FVector::ZeroVector}; //The velocity to move the target that took damage

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float knockbackStopTime {0.5}; //The amount of time to wait before stopping movement
};