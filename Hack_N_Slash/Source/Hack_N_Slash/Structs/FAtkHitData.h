#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/RootMotionSource.h"
#include "FAtkHitData.generated.h"

class UNiagaraSystem;

// Defines wether the player needs to be locked on or not to perform an attack
UENUM(BlueprintType)
enum class EHitReaction : uint8
{
    None,
    NoReact,
    Air,
    BlockBreak,
    BlockHit,
    BlockPerfect,
    BounceGround,
    BounceWall,
    Countered,
    LaunchDown,
    LaunchFwdHard,
    LaunchFwdSoftBack,
    LaunchFwdSoftFront,
    LaunchUp,
    StaggerBack,
    StaggerDownFront1,
    StaggerDownFront2,
    StaggerDownFront3,
    StaggerUpFront1,
    StaggerUpFront2,
    StaggerUpFront3,
    WallSplat
};


UENUM(BlueprintType)
enum class EKnockbackType : uint8
{
    Constant,
    MoveTo
};

// This is the data that will be passed into the custom damage system
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
    // Special
    //--------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Special Properties", meta = (Categories = "Attack.Motion."))
	FGameplayTag attackMotion;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Special Properties", meta = (ToolTip = "The hit reaction this attack will try to elicit"))
    EHitReaction attackReaction = EHitReaction::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Special Properties", meta = (Categories = "Attack.Type."))
	FGameplayTag attackType;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Special Properties", meta = (ToolTip = "Can this attack break through super armor"))
    bool bArmorBreaker = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Special Properties", meta = (ToolTip = "Special flag for an attack that can only follow a parry or perfect block"))
    bool bIsCounterFollowUp = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Special Properties", meta = (Categories = "Element."))
    TArray<FGameplayTag> elements;


    //--------------------------------
    // Numbers
    //--------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Numbers", meta = (ClampMin = 0, ClampMax = 1, Tooltip = "How much this attack aggros the target"))
    float aggroBuildup = 0.05f;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Numbers")
    float dmg = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Numbers")
    float penetration = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Numbers", meta = (ClampMin = 0, ToolTip = "Will = base poise + poise plus"))
    int poise = 0;

    //--------------------------------
    // Knockback
    //--------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Knockback", meta = (ToolTip = "Should the knockback force be a 'Constant' or 'MoveTo' Root Motion Source?"))
    EKnockbackType knockBackType = EKnockbackType::Constant;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Knockback")
    ERootMotionFinishVelocityMode velocityOnFinishMode = ERootMotionFinishVelocityMode::SetVelocity;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Knockback", meta = (EditCondition = "VelocityOnFinishMode == ERootMotionFinishVelocityMode::SetVelocity", EditConditionHides))
    FVector velocityOnFinish = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Knockback", meta = (EditCondition = "VelocityOnFinishMode == ERootMotionFinishVelocityMode::ClampVelocity", EditConditionHides, ClampMin = 0))
    float clampVelocityOnFinish = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Knockback", meta = (ClampMin = "0.0", ToolTip = "How long it'll take for the victim to cover the distance"))
    float duration = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Knockback", meta = (EditCondition = "KnockBackType == EKnockbackType::Constant", EditConditionHides, ToolTip = "Should this knockback add to existing forces or override them?"))
    bool bAdditive = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Knockback", meta = (EditCondition = "KnockBackType == EKnockbackType::Constant", EditConditionHides, ToolTip = "Local-space knockback direction"))
    FVector localDir = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Knockback", meta = (EditCondition = "KnockBackType == EKnockbackType::Constant", EditConditionHides, ClampMin = "0.0", ToolTip = "Distance the victim will be moved"))
    float distance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Knockback", meta = (EditCondition = "KnockBackType == EKnockbackType::Constant", EditConditionHides, ToolTip = "How the knockback force will behave over time"))
    UCurveFloat* strengthOverTime = nullptr;

	UPROPERTY(EditAnywhere, Category = "Knockback", meta = (EditCondition = "KnockBackType == EKnockbackType::MoveTo", EditConditionHides))
	bool bRestrictSpeedToExpected = true;

    FVector moveToLoc = FVector::ZeroVector;

    /* --------- Knockback: Ground Bounce ------*/
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Knockback", meta = (EditCondition = "AttackReaction == EHitReaction::BounceGround", EditConditionHides, ClampMin = 0))
    float gbExtraBounceHeight = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Knockback", meta = (EditCondition = "AttackReaction == EHitReaction::BounceGround", EditConditionHides))
    bool bGBAdditive = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Knockback", meta = (EditCondition = "AttackReaction == EHitReaction::BounceGround", EditConditionHides))
    UCurveFloat* gbSOT = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Knockback", meta = (EditCondition = "AttackReaction == EHitReaction::BounceGround", EditConditionHides, ClampMin = 0))
    float gbCVOF = 300.0f;

    //--------------------------------
    // Feedback
    //--------------------------------

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Feedback")
    bool bKilled = false;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Feedback")
    float dmgDealt = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Feedback")
    FVector hitImpactNormal = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Feedback")
    FVector hitImpactPoint = FVector::ZeroVector;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Feedback")
    FVector hitLoc = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback")
    USoundBase* hitSFX = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback")
    UNiagaraSystem* hitVFX = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback")
    FVector hitVFXScale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Feedback")
    EHitReaction resolvedReaction = EHitReaction::None;
};