#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "../Enums/EStickMovement.h"
#include "GameplayTagContainer.h"
#include "FPlayerAtkData.generated.h"

// Represents the player's attack graph
// Each "FPlayerAtkData" is a row in the data table that defines a single attack
// Used by the "Player Combat Component" to determine what attack to play based on the player's context

// Defines wether the player needs to be locked on or not to perform an attack
UENUM(BlueprintType)
enum class ELockRequirement : uint8
{
    Either,
    Off,
	On
};

USTRUCT(BlueprintType)
struct FPlayerAtkData : public FTableRowBase
{
	GENERATED_BODY()

	// -- Attributes of this attack--
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
    bool bUnlocked = true; // Is this attack unlocked?
    
	UPROPERTY(EditAnywhere, Category = "Attributes")
	UAnimMontage* montage = nullptr; // The montage to play

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
	FName montageSection = NAME_None; // The montage section to jump to

	UPROPERTY(EditAnywhere, Category = "Attributes", meta = (ToolTip = "Next possible attacks by rown name that this can be cancelled into. Leave blank if this attack cannot be cancelled into any other attack"))
	TArray<FName> nextAtkIDs; // All the possible attack this attack can combo into

	UPROPERTY(EditAnywhere, Category = "Attributes", meta = (ToolTip = "How much does this attack want to be selected"))
	EStickMovePriority priority = EStickMovePriority::Any; // Used to determine which attack to select if multiple attacks are valid

	
	// -- Requirements to perform this attack--
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements", meta = (Categories = "State.Movement.", ToolTip = "Movement State required on the player to perform this attack. Leave blank if it doesn't matter"))
	FGameplayTag movementState;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements", meta = (Categories = "PlayerAction.", ToolTip = "Required action for the player to perform this attack"))
	FGameplayTag actionTag;

	UPROPERTY(EditAnywhere, Category = "Requirements", meta = (ToolTip = "Forwrad? Back? Left? Right?"))
	EStickDirection lStickDirection = EStickDirection::Any;

	UPROPERTY(EditAnywhere, Category = "Requirements", meta = (ToolTip = "Circle? ForwardBack? LeftRight? This overrides stick direciton if != None"))
	EStickMotion lStickMotion = EStickMotion::None;

    UPROPERTY(EditAnywhere, Category = "Requirements", meta = (ToolTip = "Does this attack require the player to be locked on or not"))
    ELockRequirement lockRequirement = ELockRequirement::Either;
};