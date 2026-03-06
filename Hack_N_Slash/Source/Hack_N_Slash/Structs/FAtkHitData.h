// Fill out your copyright notice in the Description page of Project Settings.
#pragma once
#include "CoreMinimal.h"
#include "FAtkHitData.generated.h"

UENUM(BlueprintType)
enum class EHitReaction : uint8
{
    None,
    Light,
    Medium,
    Heavy,
    Launch,
    Knockdown
};

USTRUCT(BlueprintType)
struct FAtkHitData
{
    GENERATED_BODY()

    //Who
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	AActor* attacker {nullptr};

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

    //UPROPERTY(EditAnywhere, BlueprintReadOnly)
	//bool bCanBeBlocked {true};

    //UPROPERTY(EditAnywhere, BlueprintReadOnly)
	//bool bCanBeParried {true};

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector knockbackVelocity {FVector::ZeroVector};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "The amount of time to wait before stopping movement after knockback. Won't stop if <= 0"))
	float knockbackStopTime {0.0f};
};