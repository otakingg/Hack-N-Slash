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
    // Actor Information
    //--------------------------------

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (ToolTip = "The actor that caused the attack"))
    AActor* attacker = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (ToolTip = "The actor that directly caused damage. Could be the attacker or maybe a projectile"))
    AActor* damager = nullptr;

    //--------------------------------
    // Hit Context
    //--------------------------------

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    FVector hitLoc = FVector::ZeroVector;

    //--------------------------------
    // Attack Definition
    //--------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EAttackIntent attackIntent = EAttackIntent::Stagger;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "Can this attack break through super armor"))
    bool bArmorBreaker = false;

    UPROPERTY(EditAnywhere, meta = (ClampMin="-1", ToolTip = "Will override the base poise of the attacker. -1 means don't override"))
    int poiseOverride = -1;

    //--------------------------------
    // Special Flags
    //--------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "The attack following a parry or perfect block"))
    bool bIsCounterFollowUp = false;

    //--------------------------------
    // Damage
    //--------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0"))
    float dmgHPMult = 1.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    float dmgHP = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float dmgHPDealt = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    float penetration = 0.0f;

    //--------------------------------
    // Aggro
    //--------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "1.0", Tooltip = "How much this attack aggros the target"))
    float aggroBuildup = 0.05f;

    //--------------------------------
    // Motion Request
    //--------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "The velocity to launch the hit actor"))
    FVector motionVelocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin="0.0", ToolTip = "How long to wait before stopping the motion caused by this hit. If == 0, won't stop"))
    float timeToStop = 0.0f;

    //--------------------------------
    // OUTPUT
    //--------------------------------

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FGameplayTag resolvedReaction;
};