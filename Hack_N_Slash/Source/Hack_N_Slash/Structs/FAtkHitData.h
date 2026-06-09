#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/RootMotionSource.h"
#include "FAtkHitData.generated.h"

class UNiagaraSystem;

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

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (ToolTip = "The actor that directly caused damage. Could be the attacker or maybe a projectile spawned by the attacker"))
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

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "Should this add to existing forces or override them?"))
    bool bAdditive = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "Local-space knockback direction"))
    FVector localDir = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0", ToolTip = "Distance the victim will be moved"))
    float distance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0", ToolTip = "How long it'll take for the victim to cover the distance"))
    float duration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    ERootMotionFinishVelocityMode velocityOnFinishMode = ERootMotionFinishVelocityMode::SetVelocity;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FVector velocityOnFinish = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float clampVelocityOnFinish = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "How the knockback force will behave over time"))
    UCurveFloat* strengthOverTime = nullptr;

    //--------------------------------
    // Feedback
    //--------------------------------

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FGameplayTag resolvedReaction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    USoundBase* impactSFX = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    UNiagaraSystem* impactVFX = nullptr;
};