#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameFramework/RootMotionSource.h"
#include "RootMotSrc.generated.h"

class ULocomotionComponent;

UENUM(BlueprintType)
enum class ERootMotionType : uint8
{
    None,
    Constant,
    Jump,
    MoveTo,
    Radial
};

/**
 * Applies a Root Motion Source to the owner
 */
UCLASS()
class HACK_N_SLASH_API URootMotSrc : public UAnimNotify
{
    GENERATED_BODY()

private:
    void HandleConstant(AActor* Owner, ULocomotionComponent* LocoComp);
    void HandleJump(AActor* Owner, ULocomotionComponent* LocoComp);
    void HandleMoveTo(AActor* Owner, ULocomotionComponent* LocoComp);
	void HandleRadial(AActor* Owner, ULocomotionComponent* LocoComp);

protected:
    /* ---------------- GENERAL ---------------- */

    UPROPERTY(EditAnywhere, Category = "Root Motion")
    bool bDebug = false;

    UPROPERTY(EditAnywhere, Category = "Root Motion")
    ERootMotionType SourceType = ERootMotionType::None;

    UPROPERTY(EditAnywhere, Category = "Root Motion", meta = (ClampMin = 0.0f, Tooltip = "For Move To, 0 means duration is calculated based on distance to target, else use it"))
    float duration = 0.0f;

    UPROPERTY(EditAnywhere, Category = "Root Motion|Constant_Jump", meta = (Tooltip = "Direction the force/jump will be in. Will be normalized, so only direction matters. Zero vector means forward vector of actor will be used"))
    FVector localDir = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, Category = "Root Motion|Constant_Jump")
    float distance = 600.0f;

	UPROPERTY(EditAnywhere, Category = "Root Motion|Constant_Jump")
	ERootMotionFinishVelocityMode velocityOnFinishMode = ERootMotionFinishVelocityMode::SetVelocity;

	UPROPERTY(EditAnywhere, Category = "Root Motion|Constant_Jump", meta = (EditCondition = "VelocityOnFinishMode == ERootMotionFinishVelocityMode::SetVelocity", EditConditionHides))
	FVector velocityOnFinish = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Root Motion|Constant_Jump", meta = (EditCondition = "VelocityOnFinishMode == ERootMotionFinishVelocityMode::ClampVelocity", EditConditionHides))
	float clampVelocityOnFinish = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Root Motion|Constant_Radial")
	UCurveFloat* strengthOverTime = nullptr;

    /* ---------------- Constant ---------------- */
	UPROPERTY(EditAnywhere, Category = "Root Motion|Constant")
	bool bAdditive = false;

    /* ---------------- JUMP ---------------- */

    UPROPERTY(EditAnywhere, Category = "Root Motion|Jump")
    float height = 300.0f;


    /* ---------------- MOVE TO ---------------- */

    UPROPERTY(EditAnywhere, Category = "Root Motion|MoveTo")
    float offset = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Root Motion|MoveTo")
	bool bRestrictSpeedToExpected = true;

    UPROPERTY(EditAnywhere, Category = "Root Motion|MoveTo", meta = (ClampMin = "0.1", ToolTip = "Used to calc duration, if duration isn't specified"))
    float speed = 2500.0f;

    /* ---------------- RADIAL ---------------- */

    UPROPERTY(EditAnywhere, Category = "Root Motion|Radial")
    float radius = 0.0f;

    UPROPERTY(EditAnywhere, Category = "Root Motion|Radial", meta = (Tooltip = "Positive for push, negative for pull"))
    float strength = 0.0f;

public:
    URootMotSrc();
    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};