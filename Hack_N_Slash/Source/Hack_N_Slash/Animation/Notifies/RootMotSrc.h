#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameFramework/RootMotionSource.h"
#include "RootMotSrc.generated.h"

class ILocomotionCmdInterface;

UENUM(BlueprintType)
enum class ERootMotionType : uint8
{
    None,
    Constant,
    Jump,
    MoveTo,
    Radial
};

UCLASS()
class HACK_N_SLASH_API URootMotSrc : public UAnimNotify
{
    GENERATED_BODY()

private:
    void HandleConstant(AActor* Owner, ILocomotionCmdInterface* iLocoCmd);
    void HandleJump(AActor* Owner, ILocomotionCmdInterface* iLocoCmd);
    void HandleMoveTo(AActor* Owner, ILocomotionCmdInterface* iLocoCmd);
	void HandleRadial(AActor* Owner, ILocomotionCmdInterface* iLocoCmd);

protected:
    /* ---------------- GENERAL ---------------- */

    UPROPERTY(EditAnywhere, Category = "Root Motion")
    bool bDebug = false;

    UPROPERTY(EditAnywhere, Category = "Root Motion")
    ERootMotionType SourceType = ERootMotionType::None;

    UPROPERTY(EditAnywhere, Category = "Root Motion|Constant_Jump", meta = (Tooltip = "Direction the force/jump will be in. Will be normalized, so only direction matters. Zero vector means forward vector of actor will be used"))
    FVector direction = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, Category = "Root Motion|Constant_Jump")
    float distance = 600.0f;

    UPROPERTY(EditAnywhere, Category = "Root Motion", meta = (ClampMin = 0.0f, Tooltip = "For Move To, 0 means duration is calculated based on distance to target, else use it"))
    float duration = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Root Motion")
	bool bAdditive = false;

	UPROPERTY(EditAnywhere, Category = "Root Motion")
	ERootMotionFinishVelocityMode velocityOnFinishMode = ERootMotionFinishVelocityMode::SetVelocity;

	UPROPERTY(EditAnywhere, Category = "Root Motion")
	FVector velocityOnFinish = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Root Motion")
	float clampVelocityOnFinish = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Root Motion|Constant_Radial")
	UCurveFloat* strengthOverTime = nullptr;

    /* ---------------- JUMP ---------------- */

    UPROPERTY(EditAnywhere, Category = "Root Motion|Jump")
    float height = 300.0f;


    /* ---------------- MOVE TO ---------------- */

    UPROPERTY(EditAnywhere, Category = "Root Motion|MoveTo")
    float offset = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Root Motion|MoveTo")
	bool bRestrictSpeedToExpected = true;

    /* ---------------- RADIAL (future) ---------------- */

    UPROPERTY(EditAnywhere, Category = "Root Motion|Radial")
    float radius = 0.0f;

    UPROPERTY(EditAnywhere, Category = "Root Motion|Radial", meta = (Tooltip = "Positive for push, negative for pull"))
    float strength = 0.0f;

public:
    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};