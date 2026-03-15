#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatResolutionComponent.generated.h"

class ACharacter;
struct FAtkHitData;
class ICombatInstigator;
class UStateMachineComponent;

USTRUCT(BlueprintType)
struct FReactionPermissions
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    bool bAllowFlinch = true;

    UPROPERTY(EditAnywhere)
    bool bAllowStagger = true;

    UPROPERTY(EditAnywhere)
    bool bAllowLaunch = true;

    UPROPERTY(EditAnywhere)
    bool bAllowKnockback = true;

    UPROPERTY(EditAnywhere)
    bool bAllowKnockdown = true;
};

UENUM(BlueprintType)
enum class EVulnerabilityState : uint8
{
    Normal,
    Vulnerable,
    Immune
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HACK_N_SLASH_API UCombatResolutionComponent : public UActorComponent
{
    GENERATED_BODY()

protected:
    //--------------------------------
    // Components
    //--------------------------------

    ICombatInstigator* icombatInstigator;
    
    UPROPERTY() ACharacter* ownerChar;

    UPROPERTY() UStateMachineComponent* stateMachineComp;

    //--------------------------------
    // Reaction State
    //--------------------------------

    UPROPERTY(EditAnywhere)
    EVulnerabilityState vulnerabilityState = EVulnerabilityState::Normal;

    //--------------------------------
    // Permissions
    //--------------------------------

    UPROPERTY(EditAnywhere)
    FReactionPermissions ReactionPermissions;

    //--------------------------------
    // Vulnerable Window
    //--------------------------------

    UPROPERTY(EditAnywhere)
    float VulnerableDuration = 2.f;

    FTimerHandle VulnerableTimer;

    //--------------------------------
    // Air Juggle Limiter
    //--------------------------------

    UPROPERTY(EditAnywhere)
    int MaxAirHits = 10;

    int CurrentAirHits = 0;

    UPROPERTY(EditAnywhere)
    bool bUnlimitedJuggle = false;

    /*******************************/
    virtual void BeginPlay() override;

    //--------------------------------
    // Gates
    //--------------------------------

    /** Examples of Custom Reactions
     * Perform special attack after getting hit a certain number of times
     * Block every attack except specific kinds
     */
	UFUNCTION(BlueprintNativeEvent)
    bool ResolveCustomReaction(FAtkHitData& Hit);
	virtual bool ResolveCustomReaction_Implementation(FAtkHitData& Hit) { return false; }
    bool ResolveDefense(FAtkHitData& Hit); // Parry/Block
    bool HasArmorAgainst(const FAtkHitData& Hit);
    void ResolveReaction(FAtkHitData& Hit);

    //--------------------------------
    // Vulnerability
    //--------------------------------

    void EnterVulnerable();
    void ExitVulnerable();
    bool IsVulnerable() const;

    //--------------------------------
    // Air Juggle Control
    //--------------------------------

    bool CanAirJuggle();
    bool IsAirborne() const;
    UFUNCTION() void HandleLanded(const FHitResult& Hit);

public:
    UCombatResolutionComponent();
    void ResolveHit(FAtkHitData& Hit);
};