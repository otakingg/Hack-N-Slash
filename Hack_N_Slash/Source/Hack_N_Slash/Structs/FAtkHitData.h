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
    // Damage Source
    //--------------------------------

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Source", meta = (ToolTip = "The actor that caused the attack"))
    AActor* attacker = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Source", meta = (ToolTip = "The actor that directly caused damage. Could be the attacker or maybe a projectile spawned by the attacker"))
    AActor* damager = nullptr;

    //--------------------------------
    // Tags
    //--------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tags", meta = (Categories = "Attack.Motion."))
	FGameplayTag attackMotionTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tags", meta = (Categories = "Attack.Type."))
	FGameplayTag attackTypeTag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tags", meta = (Categories = "Element."))
    TArray<FGameplayTag> elementTags;

    //--------------------------------
    // Special
    //--------------------------------
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Special")
    EAttackIntent attackIntent = EAttackIntent::Stagger;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Special", meta = (ToolTip = "Can this attack break through super armor"))
    bool bArmorBreaker = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Special", meta = (ToolTip = "The attack following a parry or perfect block"))
    bool bIsCounterFollowUp = false;

    //--------------------------------
    // Numbers
    //--------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Numbers", meta = (ClampMin = 0, ClampMax = 1, Tooltip = "How much this attack aggros the target"))
    float aggroBuildup = 0.05f;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Numbers")
    float dmg = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Numbers", meta = (ClampMin = 0))
    float dmgMult = 1.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Numbers")
    float penetration = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Numbers", meta = (ClampMin = 0, ToolTip = "Will = base poise + poise plus"))
    int poiseFinal = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Numbers", meta = (ClampMin = 0, ToolTip = "Will be added to base poise"))
    int poisePlus = 0;

    //--------------------------------
    // Knockback
    //--------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Knockback", meta = (ToolTip = "Should this add to existing forces or override them?"))
    bool bAdditive = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Knockback", meta = (ToolTip = "Local-space knockback direction"))
    FVector localDir = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Knockback", meta = (ClampMin = "0.0", ToolTip = "Distance the victim will be moved"))
    float distance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Knockback", meta = (ClampMin = "0.0", ToolTip = "How long it'll take for the victim to cover the distance"))
    float duration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Knockback")
    ERootMotionFinishVelocityMode velocityOnFinishMode = ERootMotionFinishVelocityMode::SetVelocity;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Knockback")
    FVector velocityOnFinish = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Knockback")
    float clampVelocityOnFinish = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Knockback", meta = (ToolTip = "How the knockback force will behave over time"))
    UCurveFloat* strengthOverTime = nullptr;

    //--------------------------------
    // Feedback
    //--------------------------------

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Feedback")
    float dmgDealt = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Feedback")
    FVector hitLoc = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback")
    USoundBase* hitSFX = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback")
    UNiagaraSystem* hitVFX = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Feedback")
    FGameplayTag resolvedReaction;
};