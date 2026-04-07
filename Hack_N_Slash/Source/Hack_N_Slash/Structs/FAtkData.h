#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "../../Enums/EPlayerAction.h"
#include "FAtkData.generated.h"

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
	Any UMETA(DisplayName = "Any"),
    Neutral UMETA(DisplayName = "Neutral"),
	NotNeutral UMETA(DisplayName = "Not Neutral"),
    Forward UMETA(DisplayName = "Forward"),
    Back UMETA(DisplayName = "Back"),
    Left UMETA(DisplayName = "Left"),
    Right UMETA(DisplayName = "Right"),
	Circle UMETA(DisplayName = "Circle")
};

USTRUCT(BlueprintType)
struct FAtkData : public FTableRowBase
{
	GENERATED_BODY()

	// -- Attributes of this attack--
	UPROPERTY(EditAnywhere, Category = "Attributes")
	UAnimMontage* montage = nullptr;

	UPROPERTY(EditAnywhere, Category = "Attributes", meta = (ClampMin = 0.0f, ToolTip = "For motion warping & homing. Prevents the character form warping directly ontop of their target"))
	float warpOffset = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Attributes", meta = (ClampMin = -1.0f, ToolTip = "For motion warping & homing. Maximum range at which the character will attempt to warp towards their target. Set to -1 for no max range"))
	float warpMaxRange = -1.0f;

	UPROPERTY(EditAnywhere, Category = "Attributes", meta = (ClampMin = -1.0f, ToolTip = "For motion warping & homing. Minimum range at which the character will attempt to warp towards their target. Set to -1 for no min range"))
	float warpMinRange = -1.0f;

	UPROPERTY(EditAnywhere, Category = "Attributes", meta = (ToolTip = "Next possible attacks by rown name that this can be cancelled into. Leave blank if this attack cannot be cancelled into any other attack"))
	TArray<FName> nextAtkIDs;

	UPROPERTY(EditAnywhere, Category = "Attributes", meta = (Categories = "State.Action.Combat", ToolTip = "Combat states this attack can be canceled into"))
	TArray<FGameplayTag> cancelableCombatStates;

	// Unreal doesn't allow adding multiple tags to a tag-container in the editor...so this is the workaround
	FGameplayTagContainer cancelableCombatStateContainer;

	void FillCancelableCombatStateContainer()
	{
		for (const FGameplayTag& tag : cancelableCombatStates) cancelableCombatStateContainer.AddTag(tag);
	}


	// -- Requirements to perform this attack--
	UPROPERTY(EditAnywhere, Category = "Requirements", meta = (Categories = "State.Movement.", ToolTip = "Movement State required on the character to perform this attack. Leave blank if it doesn't matter"))
	FGameplayTag requiredMovementState;

	UPROPERTY(EditAnywhere, Category = "Requirements", meta = (Tooltip = "Required action for the player to perform this attack"))
	EPlayerAction playerAction = EPlayerAction::None;

	UPROPERTY(EditAnywhere, Category = "Requirements", meta = (ToolTip = "The motion of the left stick required to perform this attack"))
	EStickMotion lStickMotion = EStickMotion::Any;

    UPROPERTY(EditAnywhere, Category = "Requirements", meta = (ToolTip = "Does this attack require the character to be locked on or not"))
    ELockRequirement lockRequirement = ELockRequirement::Either;
};