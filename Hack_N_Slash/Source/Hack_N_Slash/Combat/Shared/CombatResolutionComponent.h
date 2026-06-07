#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "../../Enums/ECombatVulnerability.h"
#include "CombatResolutionComponent.generated.h"

class ACharacter;
class ICombatInstigator;
class UBaseCharAnimInstance;
class UStateMachineComponent;
struct FAtkHitData;

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
    UAnimMontage* countered;

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

private:
    bool IsVulnerable() const { return vulnerabilityState == ECombatVulnerability::Vulnerable; }
    bool IsAirborne() const;
    bool IsGrounded() const;

protected:
    //--------------------------------
    // References
    //--------------------------------

	UPROPERTY(Transient) UBaseCharAnimInstance* animInst = nullptr;
    UPROPERTY(Transient) ACharacter* ownerChar = nullptr;
    UPROPERTY(Transient) UStateMachineComponent* stateMachineComp = nullptr;

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
    FReactionPermissions reactionPermissions;

    //--------------------------------
    // Vulnerable Window
    //--------------------------------

    UPROPERTY(EditAnywhere, Category = "Resolution")
    float vulnerableDuration = 2.f;

    FTimerHandle TH_Vulnerable;

    //--------------------------------
    // Poise
    //--------------------------------

	UPROPERTY(EditAnywhere, Category = "Resolution", meta = (ClampMin="0"))
	int poise = 0;

    //--------------------------------
    // Air Juggle Limiter
    //--------------------------------

    UPROPERTY(EditAnywhere, Category = "Resolution")
    int maxAirHits = 10;

    UPROPERTY(VisibleAnywhere, Category = "Resolution")
    int currentAirHits = 0;

    UPROPERTY(EditAnywhere, Category = "Resolution")
    bool bUnlimitedJuggle = false;

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    //--------------------------------
    // Gates
    //--------------------------------
    bool HasHigherPoise(const FAtkHitData& Hit);
    void ResolveReaction(FAtkHitData& Hit);

    //--------------------------------
    // Air Juggle Control
    //--------------------------------

    bool CanAirJuggle();
    UFUNCTION() void HandleLanded(const FHitResult& Hit);

public:
    UCombatResolutionComponent();
    void RecieveHit(FAtkHitData& Hit);

    //--------------------------------
    // Vulnerability
    //--------------------------------

    void EnterVulnerable();
    void ExitVulnerable();
    ECombatVulnerability GetVulnerability() const { return vulnerabilityState; }

    UFUNCTION(BlueprintCallable, Category = "Combat Resolution")
    void SetVulnerability(ECombatVulnerability Vulnerability) { vulnerabilityState = Vulnerability; }

    //--------------------------------
    // Poise
    //--------------------------------

    int GetPoise() const { return poise; }
    void SetPoise(int NewPoise) { poise = FMath::Max(0, NewPoise); }

    //--------------------------------
    // Hit reactions
    //--------------------------------

    FHitMontages GetHitReactions() const;
    float PlayHitReaction(UAnimMontage* Montage = nullptr, FName Section = NAME_None);
};