#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../../Enums/ECombatVulnerability.h"
#include "CombatResolutionComponent.generated.h"

class ACharacter;
class ICombatInstigator;
struct FAtkHitData;

// Struct used for storing hit reaction montages for different types of hits
// Holds most hit reacitons, but there are some exceptions
// Exception example: Player block hit reactions are section in their block montage, which is stored in the "Player Combat Component"
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
    UAnimMontage* bounceGround;

    UPROPERTY(EditDefaultsOnly)
    UAnimMontage* bounceWall;

    UPROPERTY(EditDefaultsOnly)
    UAnimMontage* wallSplat;

    UPROPERTY(EditDefaultsOnly)
    UAnimMontage* air;

    UPROPERTY(EditDefaultsOnly)
    UAnimMontage* countered;

    UPROPERTY(EditDefaultsOnly)
    UAnimMontage* death;
};

// Struct for storing reaction permissions that decides what types of reactions are allowed for this character
// Currently not meant to change during runtime
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

    UPROPERTY(EditAnywhere)
    bool bAllowBounceGround = true;

    UPROPERTY(EditAnywhere)
    bool bAllowWallSplat = true;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HACK_N_SLASH_API UCombatResolutionComponent : public UActorComponent
{
    GENERATED_BODY()

private:
    bool EnsureReferences();
    bool IsAirborne() const;
    bool IsGrounded() const;
    bool IsVulnerable() const { return vulnerabilityState == ECombatVulnerability::Vulnerable; }
    bool HasHigherPoise(const FAtkHitData& Hit);

protected:
    //--------------------------------
    // References
    //--------------------------------

    UPROPERTY(Transient) ACharacter* ownerChar = nullptr;
    ICombatInstigator* iCmbtInst = nullptr;

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

	UPROPERTY(EditAnywhere, Category = "Resolution", meta = (ClampMin = "0", ToolTip = "If hit by an attack with lower poise, won't play a reaction (unless vulnerable)"))
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
    // Reaction Resolution
    //--------------------------------

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

    UFUNCTION(BlueprintPure, Category = "Combat Resolution")
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

    const FHitMontages& GetHitReactions() const;
};