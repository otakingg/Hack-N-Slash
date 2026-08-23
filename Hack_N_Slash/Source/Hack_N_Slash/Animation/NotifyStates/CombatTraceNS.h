// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "../Structs/FAtkHitData.h"
#include "../Structs/FSocketTrace.h"
#include "CombatTraceNS.generated.h"

UENUM(BlueprintType)
enum class ETraceTypeNS : uint8
{
    Distance,
    Socket
};

/**
 * 
 */
UCLASS()
class HACK_N_SLASH_API UCombatTraceNS : public UAnimNotifyState
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Trace")
	ETraceTypeNS traceType = ETraceTypeNS::Distance;

	UPROPERTY(EditAnywhere, Category = "Trace")
	float traceRadius;

	UPROPERTY(EditAnywhere, Category = "Trace|Distance")
	float traceDistance;

	UPROPERTY(EditAnywhere, Category = "Trace|Distance", meta = (ToolTip = "This will be added to the start location of the owner"))
	FVector traceOffset;

	UPROPERTY(EditAnywhere, Category = "Trace|Socket")
	TArray<FSocketTrace> sockets;

    //--------------------------------
    // Tags
    //--------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Tags", meta = (Categories = "Attack.Motion."))
	FGameplayTag attackMotionTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Tags", meta = (Categories = "Attack.Type."))
	FGameplayTag attackTypeTag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Tags", meta = (Categories = "Element."))
    TArray<FGameplayTag> elementTags;

    //--------------------------------
    // Special
    //--------------------------------
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Special")
    EAttackIntent attackIntent = EAttackIntent::None;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Special", meta = (ToolTip = "Can this attack break through super armor"))
    bool bArmorBreaker = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Special", meta = (ToolTip = "The attack following a parry or perfect block"))
    bool bIsCounterFollowUp = false;

    //--------------------------------
    // Numbers
    //--------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Numbers", meta = (ClampMin = 0, ClampMax = 1, Tooltip = "How much this attack aggros the target"))
    float aggroBuildup = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Numbers", meta = (ClampMin = 0, ToolTip = "How much to multiply the base damage by"))
    float dmgMult = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Numbers", meta = (ClampMin = 0, ToolTip = "Will be added to base poise"))
    int poisePlus = 0;

    //--------------------------------
    // Knockback
    //--------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Knockback", meta = (ToolTip = "Should this add to existing forces or override them?"))
    bool bAdditive = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Knockback", meta = (ToolTip = "Local-space knockback direction"))
    FVector localDir = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Knockback", meta = (ClampMin = "0.0", ToolTip = "Distance the victim will be moved"))
    float distance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Knockback", meta = (ClampMin = "0.0", ToolTip = "How long it'll take for the victim to cover the distance"))
    float duration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Knockback")
    ERootMotionFinishVelocityMode velocityOnFinishMode = ERootMotionFinishVelocityMode::SetVelocity;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Knockback")
    FVector velocityOnFinish = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Knockback")
    float clampVelocityOnFinish = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Knockback", meta = (ToolTip = "How the knockback force will behave over time"))
    UCurveFloat* strengthOverTime = nullptr;

    //--------------------------------
    // Feedback
    //--------------------------------
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Feedback")
    USoundBase* hitSFX = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Feedback")
    UNiagaraSystem* hitVFX = nullptr;

public:
	UCombatTraceNS();
    virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
