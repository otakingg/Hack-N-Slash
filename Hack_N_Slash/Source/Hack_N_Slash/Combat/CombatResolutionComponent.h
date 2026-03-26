#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "CombatResolutionComponent.generated.h"

class ACharacter;
struct FAtkHitData;
class ICharAnimInterface;
class ICombatInstigator;
class UStateMachineComponent;

USTRUCT(BlueprintType)
struct FHitMontages
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly)
    UAnimMontage* flinch;

    UPROPERTY(EditDefaultsOnly)
    UAnimMontage* stagger;

    UPROPERTY(EditDefaultsOnly)
    UAnimMontage* launch;

    UPROPERTY(EditDefaultsOnly)
    UAnimMontage* knockBack;

    UPROPERTY(EditDefaultsOnly)
    UAnimMontage* knockDown;

    UPROPERTY(EditDefaultsOnly)
    UAnimMontage* airStagger;

    UPROPERTY(EditDefaultsOnly)
    UAnimMontage* death;

    UPROPERTY(EditDefaultsOnly)
    UAnimMontage* deathLaunch;

    UPROPERTY(EditDefaultsOnly)
    UAnimMontage* deathKnockBack;

    UPROPERTY(EditDefaultsOnly)
    UAnimMontage* deathKnockDown;

    UPROPERTY(EditDefaultsOnly)
    UAnimMontage* deathAirStagger;
};

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
    // Components/Interfaces
    //--------------------------------

	ICharAnimInterface* iParentAnimInst;
	//ICharAnimInterface* iChildAnimInst;
    ICombatInstigator* iCombatInstigator;
    
    UPROPERTY() ACharacter* ownerChar;

    UPROPERTY() UStateMachineComponent* stateMachineComp;

    //--------------------------------
    // Reaction State
    //--------------------------------

    UPROPERTY(EditAnywhere)
    EVulnerabilityState vulnerabilityState = EVulnerabilityState::Normal;

    //--------------------------------
    // Reactions
    //--------------------------------

    UPROPERTY(EditDefaultsOnly)
    FHitMontages hitReactions;

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
    UFUNCTION() void HandleLanded(const FHitResult& Hit);

    //--------------------------------
    // Queries
    //--------------------------------
    bool IsAirborne() const;
    bool IsGrounded() const;

public:
    UCombatResolutionComponent();
    void ResolveHit(FAtkHitData& Hit);

    FHitMontages GetHitReactions() const;
    float PlayHitReaction(UAnimMontage* Montage = nullptr, FName Section = NAME_None);
};