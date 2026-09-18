#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "../Structs/FAtkHitData.h"
#include "../Structs/FSocketTrace.h"
#include "CombatTrace.generated.h"

UENUM(BlueprintType)
enum class ETraceTypeN : uint8
{
    Forward,
    Socket
};

/**
 * Performs a trace using the Combat Trace Component
 */
UCLASS()
class HACK_N_SLASH_API UCombatTrace : public UAnimNotify
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Trace")
	ETraceTypeN traceType = ETraceTypeN::Socket;

	UPROPERTY(EditAnywhere, Category = "Trace", meta = (ClampMin = 0))
	float traceRadius;

	UPROPERTY(EditAnywhere, Category = "Trace|Forward", meta = (ClampMin = 0))
	float traceDistance;

	UPROPERTY(EditAnywhere, Category = "Trace|Forward", meta = (ToolTip = "This will be added to the start location of the owner"))
	FVector traceOffset;

	UPROPERTY(EditAnywhere, Category = "Trace|Socket")
	TArray<FSocketTrace> sockets;

    //--------------------------------
    // Special
    //--------------------------------
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Special Properties", meta = (Categories = "Attack.Motion."))
	FGameplayTag attackMotion;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Special Properties")
    EAttackIntent attackIntent = EAttackIntent::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Special Properties", meta = (Categories = "Attack.Type."))
	FGameplayTag attackType;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Special Properties", meta = (ToolTip = "Can this attack break through super armor"))
    bool bArmorBreaker = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Special Properties", meta = (ToolTip = "Special flag for an attack that can only follow a parry or perfect block"))
    bool bIsCounterFollowUp = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Special Properties", meta = (Categories = "Element."))
    TArray<FGameplayTag> elements;

    //--------------------------------
    // Numbers
    //--------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Numbers", meta = (ClampMin = 0, ClampMax = 1, Tooltip = "How much this attack aggros the target"))
    float aggroBuildup = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Numbers", meta = (ClampMin = 0, ToolTip = "How much to multiply the base damage by"))
    float dmgMult = 1.0f;

    //--------------------------------
    // Knockback
    //--------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Knockback", meta = (ToolTip = "Should the knockback force be a 'Constant' or 'MoveTo' Root Motion Source?"))
    EKnockbackType knockBackType = EKnockbackType::Constant;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Knockback")
    ERootMotionFinishVelocityMode velocityOnFinishMode = ERootMotionFinishVelocityMode::MaintainLastRootMotionVelocity;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Knockback", meta = (EditCondition = "VelocityOnFinishMode == ERootMotionFinishVelocityMode::SetVelocity", EditConditionHides))
    FVector velocityOnFinish = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Knockback", meta = (EditCondition = "VelocityOnFinishMode == ERootMotionFinishVelocityMode::ClampVelocity", EditConditionHides, ClampMin = 0))
    float clampVelocityOnFinish = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Knockback", meta = (ClampMin = 0, ToolTip = "How long it'll take for the victim to cover the distance"))
    float duration = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Knockback", meta = (EditCondition = "KnockBackType == EKnockbackType::Constant", EditConditionHides, ToolTip = "Should this add to existing forces or override them?"))
    bool bAdditive = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Knockback", meta = (EditCondition = "KnockBackType == EKnockbackType::Constant", EditConditionHides, ToolTip = "How the knockback force will behave over time"))
    UCurveFloat* strengthOverTime = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Knockback", meta = (EditCondition = "KnockBackType == EKnockbackType::Constant", EditConditionHides, ToolTip = "Local-space knockback direction. Will be normalized so only direction matters"))
    FVector localDir = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Knockback", meta = (EditCondition = "KnockBackType == EKnockbackType::Constant", EditConditionHides, ClampMin = 0, ToolTip = "Distance the victim will be moved"))
    float distance = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Hit Data|Knockback", meta = (EditCondition = "KnockBackType == EKnockbackType::MoveTo", EditConditionHides))
	bool bRestrictSpeedToExpected = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Knockback", meta = (EditCondition = "KnockBackType == EKnockbackType::MoveTo", EditConditionHides, ToolTip = "Will be added to the location of the attacker, which will be the move to location"))
    FVector moveToOffset = {100.0f, 0.0f, 100.0f};

    /* --------- Knockback: Ground Bounce ------*/
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Knockback", meta = (EditCondition = "AttackIntent == EAttackIntent::BounceGround", EditConditionHides, ClampMin = 0))
    float gbExtraBounceHeight = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Knockback", meta = (EditCondition = "AttackIntent == EAttackIntent::BounceGround", EditConditionHides))
    bool bGBAdditive = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Knockback", meta = (EditCondition = "AttackIntent == EAttackIntent::BounceGround", EditConditionHides))
    UCurveFloat* gbSOT = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Knockback", meta = (EditCondition = "AttackIntent == EAttackIntent::BounceGround", EditConditionHides, ClampMin = 0))
    float gbCVOF = 300.0f;

    //--------------------------------
    // Feedback
    //--------------------------------
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Feedback")
    USoundBase* hitSFX = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Feedback")
    UNiagaraSystem* hitVFX = nullptr;
	
public:
	UCombatTrace();
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
};
