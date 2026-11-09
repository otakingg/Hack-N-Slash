#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "../Structs/FAtkHitData.h"
#include "../Structs/FHitBoxShape.h"
#include "CombatTraceNS.generated.h"

/**
 * Performs a trace using the Combat Trace Component
 */
UCLASS()
class HACK_N_SLASH_API UCombatTraceNS : public UAnimNotifyState
{
	GENERATED_BODY()

protected:
    UPROPERTY(EditAnywhere, Category = "Trace")
    bool bDebug = false;

	UPROPERTY(EditAnywhere, Category = "Trace")
	TArray<FHitBoxShape> hitBoxes;

    //--------------------------------
    // Special
    //--------------------------------

    UPROPERTY(EditAnywhere, Category = "Hit Data|Special Properties", meta = (ClampMin = 0, ToolTip = "How much to multiply the base damage by"))
    float dmgMult = 1.0f;
    
	UPROPERTY(EditAnywhere, Category = "Hit Data|Special Properties", meta = (Categories = "Attack.Motion."))
	FGameplayTag attackMotion;

    UPROPERTY(EditAnywhere, Category = "Hit Data|Special Properties", meta = (ToolTip = "The hit reaction this attack will try to elicit"))
    EHitReaction attackReaction = EHitReaction::None;

	UPROPERTY(EditAnywhere, Category = "Hit Data|Special Properties", meta = (Categories = "Attack.Type."))
	FGameplayTag attackType;
    
    UPROPERTY(EditAnywhere, Category = "Hit Data|Special Properties", meta = (ToolTip = "Can this attack break through super armor"))
    bool bArmorBreaker = false;

    UPROPERTY(EditAnywhere, Category = "Hit Data|Special Properties", meta = (ToolTip = "Special flag for an attack that can only follow a parry or perfect block"))
    bool bIsCounterFollowUp = false;

    UPROPERTY(EditAnywhere, Category = "Hit Data|Special Properties", meta = (Categories = "Element."))
    TArray<FGameplayTag> elements;

    //--------------------------------
    // Knockback
    //--------------------------------

    UPROPERTY(EditAnywhere, Category = "Hit Data|Knockback", meta = (ToolTip = "Should the knockback force be a 'Constant' or 'MoveTo' Root Motion Source?"))
    EKnockbackType knockBackType = EKnockbackType::Constant;

    UPROPERTY(EditAnywhere, Category = "Hit Data|Knockback")
    ERootMotionFinishVelocityMode velocityOnFinishMode = ERootMotionFinishVelocityMode::MaintainLastRootMotionVelocity;

    UPROPERTY(EditAnywhere, Category = "Hit Data|Knockback", meta = (EditCondition = "VelocityOnFinishMode == ERootMotionFinishVelocityMode::SetVelocity", EditConditionHides))
    FVector velocityOnFinish = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, Category = "Hit Data|Knockback", meta = (EditCondition = "VelocityOnFinishMode == ERootMotionFinishVelocityMode::ClampVelocity", EditConditionHides, ClampMin = 0))
    float clampVelocityOnFinish = 0.0f;

    UPROPERTY(EditAnywhere, Category = "Hit Data|Knockback", meta = (ClampMin = 0, ToolTip = "How long it'll take for the victim to cover the distance"))
    float duration = 0.1f;

    UPROPERTY(EditAnywhere, Category = "Hit Data|Knockback", meta = (EditCondition = "KnockBackType == EKnockbackType::Constant", EditConditionHides, ToolTip = "Should this add to existing forces or override them?"))
    bool bAdditive = false;

    UPROPERTY(EditAnywhere, Category = "Hit Data|Knockback", meta = (EditCondition = "KnockBackType == EKnockbackType::Constant", EditConditionHides, ToolTip = "How the knockback force will behave over time"))
    UCurveFloat* strengthOverTime = nullptr;

    UPROPERTY(EditAnywhere, Category = "Hit Data|Knockback", meta = (EditCondition = "KnockBackType == EKnockbackType::Constant", EditConditionHides, ToolTip = "Local-space knockback direction. Will be normalized so only direction matters"))
    FVector localDir = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, Category = "Hit Data|Knockback", meta = (EditCondition = "KnockBackType == EKnockbackType::Constant", EditConditionHides, ClampMin = 0, ToolTip = "Distance the victim will be moved"))
    float distance = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Hit Data|Knockback", meta = (EditCondition = "KnockBackType == EKnockbackType::MoveTo", EditConditionHides))
	bool bRestrictSpeedToExpected = true;

    UPROPERTY(EditAnywhere, Category = "Hit Data|Knockback", meta = (EditCondition = "KnockBackType == EKnockbackType::MoveTo", EditConditionHides, ToolTip = "Will be added to the location of the attacker, which will be the move to location"))
    FVector moveToOffset = {100.0f, 0.0f, 100.0f};

    /* --------- Knockback: Ground Bounce ------*/
    UPROPERTY(EditAnywhere, Category = "Hit Data|Knockback", meta = (EditCondition = "AttackReaction == EHitReaction::BounceGround", EditConditionHides, ClampMin = 0))
    float gbExtraBounceHeight = 100.0f;

    UPROPERTY(EditAnywhere, Category = "Hit Data|Knockback", meta = (EditCondition = "AttackReaction == EHitReaction::BounceGround", EditConditionHides))
    bool bGBAdditive = false;

    UPROPERTY(EditAnywhere, Category = "Hit Data|Knockback", meta = (EditCondition = "AttackReaction == EHitReaction::BounceGround", EditConditionHides))
    UCurveFloat* gbSOT = nullptr;
    
    UPROPERTY(EditAnywhere, Category = "Hit Data|Knockback", meta = (EditCondition = "AttackReaction == EHitReaction::BounceGround", EditConditionHides, ClampMin = 0))
    float gbCVOF = 300.0f;

    //--------------------------------
    // Feedback
    //--------------------------------
    UPROPERTY(EditAnywhere, Category = "Hit Data|Feedback")
    USoundBase* hitSFX = nullptr;

    UPROPERTY(EditAnywhere, Category = "Hit Data|Feedback")
    UNiagaraSystem* hitVFX = nullptr;

    UPROPERTY(EditAnywhere, Category = "Hit Data|Feedback")
    FVector hitVFXScale = FVector(1.0f, 1.0f, 1.0f);
    
public:
	UCombatTraceNS();
    virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
