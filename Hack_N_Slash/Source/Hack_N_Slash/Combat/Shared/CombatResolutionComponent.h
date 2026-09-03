#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatResolutionComponent.generated.h"

class ACharacter;
class UCharacterMovementComponent;
class UStateMachineComponent;
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
    bool HasHigherPoise(const FAtkHitData& Hit);

protected:
    //--------------------------------
    // References
    //--------------------------------

    UPROPERTY(Transient) ACharacter* ownerChar = nullptr;
    UPROPERTY(Transient) UCharacterMovementComponent* moveComp = nullptr;
    UPROPERTY(Transient) UStateMachineComponent* stateMachineComp = nullptr;

    //--------------------------------
    // Immunity
    //--------------------------------

    UPROPERTY(EditAnywhere, Category = "Resolution")
    bool bImmune = false;

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
    // Poise
    //--------------------------------

	UPROPERTY(EditAnywhere, Category = "Resolution", meta = (ClampMin = "0", ToolTip = "If hit by an attack with lower poise, won't play a reaction (unless vulnerable)"))
	int poiseBase = 0;

    UPROPERTY(VisibleAnywhere, Category = "Resolution", meta = (ClampMin = "0", ToolTip = "Poise used in reaction calculation"))
    int poiseCalc = 0;

    UPROPERTY(VisibleAnywhere, Category = "Resolution", meta = (ToolTip = "Is the poise value currently overriden?"))
    bool bPoiseOverriden = false;

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
    // Immunity
    //--------------------------------

    UFUNCTION(BlueprintCallable, Category = "Resolution")
    void SetImmunity(bool bFlag) { bImmune = bFlag; }
    bool IsImmune() const { return bImmune; }

    //--------------------------------
    // Poise
    //--------------------------------

    int GetPoiseBase() const { return poiseBase;}
    void SetPoiseBase(int NewPoise) { poiseBase = FMath::Max(0, NewPoise);}

    int GetPoiseCalc() const { return poiseCalc; }
    void SetPoiseCalc(int NewPoise)
    {
        if (bPoiseOverriden) return;
        else
        {
            poiseCalc = FMath::Max(0, NewPoise);
            bPoiseOverriden = true;
        }
    }
    void ResetPoiseCalc()
    {
        poiseCalc = poiseBase;
        bPoiseOverriden = false;
    }

    //--------------------------------
    // Hit reactions
    //--------------------------------

    const FHitMontages& GetHitReactions() const;
};