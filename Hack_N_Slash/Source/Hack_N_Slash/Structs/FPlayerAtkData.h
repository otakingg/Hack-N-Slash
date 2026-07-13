#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "FPlayerAtkData.generated.h"

UENUM(BlueprintType)
enum class ELockRequirement : uint8
{
    Either,
    Off,
	On
};

UENUM(BlueprintType)
enum class EStickMotion : uint8
{
    Neutral,
    Any,
    NotNeutral,

    Forward,
    ForwardRight,
    Right,
    BackRight,
    Back,
    BackLeft,
    Left,
    ForwardLeft,

    BackToForward,
    ForwardToBack,
    LeftToRight,
    RightToLeft,

    QCF,        // quarter-circle forward
    QCB,        // quarter-circle back
    HCF,        // half-circle forward
    HCB,		// half-circle back
    FC,			// full-circle
};

USTRUCT(BlueprintType)
struct FPlayerAtkData : public FTableRowBase
{
	GENERATED_BODY()

	// -- Attributes of this attack--
	UPROPERTY(EditAnywhere, Category = "Attributes")
	UAnimMontage* montage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
	FName montageSection = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack", meta = (Categories = "Attack."))
    TArray<FGameplayTag> attackTags;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack", meta = (Categories = "Element."))
    TArray<FGameplayTag> elementTags;

	UPROPERTY(EditAnywhere, Category = "Attributes", meta = (ClampMin = 0.0f, ToolTip = "For warping. Prevents the player from warping directly ontop of their target. 0 means don't warp transationally"))
	float warpOffset = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Attributes", meta = (ToolTip = "For warping. Should this attack ignore the free flow rules when it comes to gathering warp data"))
	bool bIgnoreFreeFlowRules = false;

	UPROPERTY(EditAnywhere, Category = "Attributes", meta = (ToolTip = "Next possible attacks by rown name that this can be cancelled into. Leave blank if this attack cannot be cancelled into any other attack"))
	TArray<FName> nextAtkIDs;

	
	// -- Requirements to perform this attack--
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
    bool bUnlocked = true;
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements", meta = (Categories = "State.Movement.", ToolTip = "Movement State required on the player to perform this attack. Leave blank if it doesn't matter"))
	FGameplayTag movementState;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements", meta = (Categories = "PlayerAction.", ToolTip = "Required action for the player to perform this attack"))
	FGameplayTag actionTag;

	UPROPERTY(EditAnywhere, Category = "Requirements", meta = (ToolTip = "The motion of the left stick required to perform this attack"))
	EStickMotion lStickMotion = EStickMotion::Any;

    UPROPERTY(EditAnywhere, Category = "Requirements", meta = (ToolTip = "Does this attack require the player to be locked on or not"))
    ELockRequirement lockRequirement = ELockRequirement::Either;
};