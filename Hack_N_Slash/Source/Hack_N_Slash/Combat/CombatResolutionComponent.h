#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../Structs/FAtkHitData.h"
#include "CombatResolutionComponent.generated.h"

UENUM(BlueprintType)
enum class EReactionState : uint8
{
    Vulnerable = 0,     // Forced reactions / combo window
    Normal = 1,         // Standard reactions allowed
    SuperArmor = 2,     // Light attacks don't react
    HyperArmor = 3,     // No reactions unless countered
    Immune = 4          // Completely immune
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HACK_N_SLASH_API UCombatResolutionComponent : public UActorComponent
{
    GENERATED_BODY()

private:
    UPROPERTY() ACharacter* ownerChar {nullptr};
    UPROPERTY() class UStateMachineComponent* stateMachineComp {nullptr};
    //----------------------------------
    // Internal State
    //----------------------------------

    FTimerHandle TH_Vulnerable;

    //----------------------------------
    // Resolution Steps
    //----------------------------------

    bool Step_Parry(FAtkHitData& HitData);
    bool Step_Block(FAtkHitData& HitData);
    bool Step_Armor(FAtkHitData& HitData);

    void Step_Posture(FAtkHitData& HitData);
    void Step_Reaction(FAtkHitData& HitData);

    //----------------------------------
    // State Changes
    //----------------------------------

    void EnterVulnerable();
    UFUNCTION() void ExitVulnerable();
    bool IsVulnerable() const;

    //bool CanBeLaunched() const;

    void Fall();
    UFUNCTION() void ResetAirState(const FHitResult& Hit); //Call when landing
    bool IsAirborne() const;

protected:
    //----------------------------------
    // Reaction State
    //----------------------------------

    UPROPERTY(EditAnywhere, Category="Combat")
    EReactionState reactionState {EReactionState::Normal};

    //----------------------------------
    // Armor
    //----------------------------------

    UPROPERTY(EditDefaultsOnly, Category="Combat")
    EAtkType armorType {EAtkType::Light};

    //----------------------------------
    // Posture
    //----------------------------------

    UPROPERTY(EditDefaultsOnly, Category="Posture")
    bool bUsePosture {true};

    UPROPERTY(VisibleAnywhere, Category="Posture")
    float postureCurrent {100.f};

    UPROPERTY(EditDefaultsOnly, Category="Posture")
    float postureMax {100.f};

    //----------------------------------
    // Vulnerable
    //----------------------------------

    UPROPERTY(EditAnywhere, Category="Posture")
    float vulnerableDuration {1.25f};

    //----------------------------------
    // Air Rules
    //----------------------------------
    UPROPERTY(EditAnywhere, Category="Air")
    bool bLaunchImmune {false};

    UPROPERTY(VisibleAnywhere, Category="Air")
    bool bAirTimerExpired {false};

    UPROPERTY(VisibleAnywhere, Category="Air")
    float airTime {0.0f};

    UPROPERTY(EditAnywhere, Category="Air")
    float maxAirTime {2.5f};

public:
    UCombatResolutionComponent();

    void BeginPlay();
    void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction);

    void ResolveHit(FAtkHitData& HitData);
};