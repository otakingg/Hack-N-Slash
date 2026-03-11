#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "FAtkHitData.generated.h"

UENUM(BlueprintType)
enum class EAtkType : uint8
{
    Light = 0,
    Heavy = 1,
    Massive = 2
};

UENUM(BlueprintType)
enum class EAtkReactionType : uint8
{
    Neutral,
    Launch,
    Knockdown,
    Knockback
};

USTRUCT(BlueprintType)
struct FAtkHitData
{
    GENERATED_BODY()

    //----------------------------------
    // Attacker Info
    //----------------------------------

    UPROPERTY() AActor* attacker {nullptr};

    //----------------------------------
    // Hit Context
    //----------------------------------

    UPROPERTY() FVector hitLoc {FVector::ZeroVector};
    UPROPERTY() FVector hitDir {FVector::ZeroVector};

    //----------------------------------
    // Attack Definition
    //----------------------------------

    UPROPERTY(EditAnywhere)
    EAtkType atkType {EAtkType::Light};

    UPROPERTY(EditAnywhere)
    EAtkReactionType reactionType {EAtkReactionType::Neutral};

    UPROPERTY(EditAnywhere)
    bool bIsCounter {false};

    //----------------------------------
    // Damage
    //----------------------------------

    UPROPERTY(EditAnywhere)
    float dmgHP {0.f};

    UPROPERTY(EditAnywhere)
    float dmgPosture {0.f};

    UPROPERTY(EditAnywhere)
    float penetration {0.f};

    //----------------------------------
    // Defense Flags
    //----------------------------------

    UPROPERTY(EditAnywhere)
    bool bCanBeBlocked {true};

    UPROPERTY(EditAnywhere)
    bool bCanBeParried {true};

    UPROPERTY(EditAnywhere, meta=(ClampMin="0.0", Tooltip="If defendable, what level of defense is required. EX: In NG4, power attacks can only be defended in blood raven form. Might eventually replace this with a tag or enum"))
    int defenseRequirementLvl {0};

    //----------------------------------
    // Motion
    //----------------------------------

    UPROPERTY(EditAnywhere)
    FVector knockbackVelocity {FVector::ZeroVector};

    UPROPERTY(EditAnywhere)
    float knockbackStopTime {0.f};

    //----------------------------------
    // Output
    //----------------------------------

    UPROPERTY(VisibleAnywhere)
    FGameplayTag resolvedReaction;
};