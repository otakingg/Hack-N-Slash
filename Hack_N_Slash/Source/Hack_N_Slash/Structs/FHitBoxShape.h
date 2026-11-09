#pragma once

#include "CoreMinimal.h"
#include "FHitBoxShape.generated.h"

UENUM(BlueprintType)
enum class EHitBoxShapeType : uint8
{
    ActorDirection, // Sphere extends along the actors direction based on chosen axis
    SingleBone,  // 1 bone, sphere extends along a chosen axis
    BoneToBone   // Capsule stretches between two bones
};

USTRUCT(BlueprintType)
struct FHitBoxShape
{
	GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    EHitBoxShapeType shape = EHitBoxShapeType::SingleBone;

    UPROPERTY(EditAnywhere, meta = (EditCondition = "Shape == EHitBoxShapeType::SingleBone || Shape == EHitBoxShapeType::BoneToBone", EditConditionHides))
    FName startBone; // SingleBone/BoneToBone: Starting bone

    UPROPERTY(EditAnywhere, meta = (EditCondition = "Shape == EHitBoxShapeType::BoneToBone", EditConditionHides))
    FName endBone; // BoneToBone only

    UPROPERTY(EditAnywhere, meta = (ClampMin = 0))
    float radius = 0.0f;

    UPROPERTY(EditAnywhere, meta = (ToolTip = "Use numbers < 0 to extend back, left, & down", EditCondition = "Shape == EHitBoxShapeType::SingleBone || Shape == EHitBoxShapeType::ActorDirection", EditConditionHides))
    float length = 0.0f; // SingleBone/ActorDirection: length to extend along chosen axis

    UPROPERTY(EditAnywhere, meta = (ClampMin = 0, EditCondition = "Shape == EHitBoxShapeType::BoneToBone", EditConditionHides))
    float extraLength = 0.0f; // BoneToBone: padding past the end bone

    UPROPERTY(EditAnywhere, meta = (EditCondition = "Shape == EHitBoxShapeType::SingleBone || Shape == EHitBoxShapeType::ActorDirection", EditConditionHides))
    FVector localOffset = FVector::ZeroVector; // SingleBone/ActorDirection: Local offset from the bone or actor's location

    UPROPERTY(EditAnywhere, meta = (EditCondition = "Shape == EHitBoxShapeType::SingleBone || Shape == EHitBoxShapeType::ActorDirection", EditConditionHides))
    TEnumAsByte<EAxis::Type> axis = EAxis::Z; // SingleBone/ActorDirection: Axis to extend along
};