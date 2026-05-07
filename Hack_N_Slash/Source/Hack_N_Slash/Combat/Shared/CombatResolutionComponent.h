#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "../../Enums/ECombatVulnerability.h"
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
    UAnimMontage* blockHit;

    UPROPERTY(EditDefaultsOnly)
    UAnimMontage* blockBreak;

    UPROPERTY(EditDefaultsOnly)
    UAnimMontage* death;
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

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HACK_N_SLASH_API UCombatResolutionComponent : public UActorComponent
{
    GENERATED_BODY()

protected:
    //--------------------------------
    // Components/Interfaces
    //--------------------------------

	ICharAnimInterface* iAnimInst;
    ICombatInstigator* iCombatInstigator;
    
    UPROPERTY() ACharacter* ownerChar;

    UPROPERTY() UStateMachineComponent* stateMachineComp;

    //--------------------------------
    // Reaction State
    //--------------------------------

    UPROPERTY(EditAnywhere, Category = "Resolution")
    ECombatVulnerability vulnerabilityState = ECombatVulnerability::Normal;

    //--------------------------------
    // Reactions
    //--------------------------------

    UPROPERTY(EditDefaultsOnly, Category = "Resolution")
    FHitMontages hitReactions;

    //--------------------------------
    // Permissions
    //--------------------------------

    UPROPERTY(EditAnywhere, Category = "Resolution")
    FReactionPermissions ReactionPermissions;

    //--------------------------------
    // Vulnerable Window
    //--------------------------------

    UPROPERTY(EditAnywhere, Category = "Resolution")
    float VulnerableDuration = 2.f;

    FTimerHandle VulnerableTimer;

    //--------------------------------
    // Air Juggle Limiter
    //--------------------------------

    UPROPERTY(EditAnywhere, Category = "Resolution")
    int MaxAirHits = 10;

    UPROPERTY(VisibleAnywhere, Category = "Resolution")
    int CurrentAirHits = 0;

    UPROPERTY(EditAnywhere, Category = "Resolution")
    bool bUnlimitedJuggle = false;

    /*******************************/
    virtual void BeginPlay() override;

    //--------------------------------
    // Gates
    //--------------------------------
    bool HasHigherPowerLvl(const FAtkHitData& Hit);
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
    //--------------------------------
    // Power Level
    //--------------------------------
	UPROPERTY(EditAnywhere, Category = "Resolution", meta = (ClampMin="0"))
	int powerLvl = 0;

	UPROPERTY(EditAnywhere, Category = "Resolution", meta = (ClampMin="1"))
	int powerLvlMax = 3;
    
    UCombatResolutionComponent();
    void ResolveHit(FAtkHitData& Hit);

    FHitMontages GetHitReactions() const;
    float PlayHitReaction(UAnimMontage* Montage = nullptr, FName Section = NAME_None);

    ECombatVulnerability GetVulnerability() const { return vulnerabilityState; }
    void SetVulnerability(ECombatVulnerability Vulnerability) { vulnerabilityState = Vulnerability; }
};