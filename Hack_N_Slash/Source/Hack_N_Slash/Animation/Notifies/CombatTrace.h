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
 * 
 */
UCLASS()
class HACK_N_SLASH_API UCombatTrace : public UAnimNotify
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Trace")
	ETraceTypeN traceType = ETraceTypeN::Socket;

	UPROPERTY(EditAnywhere, Category = "Trace")
	float traceRadius;

	UPROPERTY(EditAnywhere, Category = "Trace|Forward")
	float traceDistance;

	UPROPERTY(EditAnywhere, Category = "Trace|Forward", meta = (ToolTip = "This will be added to the start location of the owner"))
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

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Knockback", meta = (ToolTip = "How the knockback force will behave over time"))
    UCurveFloat* strengthOverTime = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Knockback", meta = (ToolTip = "Local-space knockback direction. Will be normalized so only direction matters"))
    FVector localDir = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Knockback", meta = (ClampMin = "0.0", ToolTip = "Distance the victim will be moved"))
    float distance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Knockback", meta = (ClampMin = "0.0", ToolTip = "How long it'll take for the victim to cover the distance"))
    float duration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Knockback")
    ERootMotionFinishVelocityMode velocityOnFinishMode = ERootMotionFinishVelocityMode::SetVelocity;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Knockback", meta = (EditCondition = "VelocityOnFinishMode == ERootMotionFinishVelocityMode::SetVelocity", EditConditionHides))
    FVector velocityOnFinish = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Data|Knockback", meta = (EditCondition = "VelocityOnFinishMode == ERootMotionFinishVelocityMode::ClampVelocity", EditConditionHides))
    float clampVelocityOnFinish = 0.0f;

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
