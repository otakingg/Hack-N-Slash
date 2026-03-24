#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "FAtkHitData.generated.h"

UENUM(BlueprintType)
enum class EAttackIntent : uint8
{
    None,
    Flinch,
    Stagger,
    Launch,
    Knockback,
    Knockdown
};

USTRUCT(BlueprintType)
struct FAtkHitData
{
    GENERATED_BODY()

    //--------------------------------
    // Attacker
    //--------------------------------

    UPROPERTY() AActor* attacker = nullptr;

    //--------------------------------
    // Hit Context
    //--------------------------------

    UPROPERTY() FVector hitLoc = FVector::ZeroVector;

    //--------------------------------
    // Attack Definition
    //--------------------------------

    UPROPERTY(EditAnywhere)
    EAttackIntent attackIntent = EAttackIntent::Stagger;

    UPROPERTY(EditAnywhere, meta = (ClampMin="0", ToolTip = "Will be added to the base power level of the attacker"))
    int powerLevelAddition = 0;

    UPROPERTY(EditAnywhere, meta = (ClampMin="-1", ClampMax="2", ToolTip = "Will override the base power level of the attacker. -1 means don't override"))
    int powerLevelOverride = -1;

    //--------------------------------
    // Special Flags
    //--------------------------------

    UPROPERTY(EditAnywhere, meta = (ToolTip = "The attack following a parry or perfect block"))
    bool bIsCounterFollowUp = false;

    //--------------------------------
    // Damage
    //--------------------------------

    UPROPERTY(EditAnywhere, meta=(ClampMin="0.0"))
    float dmgHPMult = 1.0f;
    float dmgHP = 0.0f;
    float dmgHPDealt = 0.0f;
    float penetration = 0.0f;

    //--------------------------------
    // Motion Request
    //--------------------------------

    UPROPERTY(EditAnywhere, meta = (ToolTip = "The velocity to launch the hit actor"))
    FVector motionVelocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, meta = (ClampMin="0.0", ToolTip = "How long to wait before stopping the motion caused by this hit. If == 0, won't stop"))
    float timeToStop = 0.0f;

    //--------------------------------
    // OUTPUT
    //--------------------------------

    UPROPERTY(VisibleAnywhere)
    FGameplayTag resolvedReaction;
};