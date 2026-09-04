#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatResolutionComponent.generated.h"

class ACharacter;
class UCharacterMovementComponent;
class UStateMachineComponent;
struct FAtkHitData;

// Struct used for storing hit reaction montages for different types of hits
// Holds most hit reactions, but there are some exceptions
// Exception example: Player block hit reactions are sections in their block montage, which is stored in the "Player Combat Component"
USTRUCT(BlueprintType)
struct FHitMontages
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    UAnimMontage* flinch;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    UAnimMontage* stagger;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    UAnimMontage* launch;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    UAnimMontage* knockBack;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    UAnimMontage* knockDown;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    UAnimMontage* bounceGround;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    UAnimMontage* bounceWall;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    UAnimMontage* wallSplat;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    UAnimMontage* air;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    UAnimMontage* countered;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
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

    //--------------------------------
    // Poise
    //--------------------------------

    bool HasHigherPoise(const FAtkHitData& Hit) const;

    //--------------------------------
    // Air Juggle
    //--------------------------------

    bool CanAirJuggle() const { return bUnlimitedJuggle || (currentAirHits < maxAirHits); }
    UFUNCTION() void HandleLanded(const FHitResult& Hit) { currentAirHits = 0; }

protected:
    //--------------------------------
    // References
    //--------------------------------

    UPROPERTY(Transient) ACharacter* ownerChar = nullptr;
    UPROPERTY(Transient) UCharacterMovementComponent* moveComp = nullptr;
    UPROPERTY(Transient) UStateMachineComponent* stateMachineComp = nullptr;

    //--------------------------------
    // Reactions
    //--------------------------------

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Resolution")
    FHitMontages hitReactions;

    //--------------------------------
    // Permissions
    //--------------------------------

    UPROPERTY(EditAnywhere, Category = "Resolution")
    FReactionPermissions reactionPermissions;

    //--------------------------------
    // Immunity
    //--------------------------------

    UPROPERTY(EditAnywhere, Category = "Resolution")
    bool bImmune = false;

    //--------------------------------
    // Poise
    //--------------------------------

	UPROPERTY(EditAnywhere, Category = "Resolution", meta = (ClampMin = "0", ToolTip = "If hit by an attack with lower poise, won't play a reaction"))
	int poiseBase = 0;

    UPROPERTY(VisibleAnywhere, Category = "Resolution", meta = (ClampMin = "-1", ToolTip = "Poise used in reaction calculation"))
    int poiseCalc = 0;

    UPROPERTY(VisibleAnywhere, Category = "Resolution", meta = (ToolTip = "Is the poise value currently overriden?"))
    bool bPoiseOverriden = false;

    //--------------------------------
    // Air Juggle
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

public:
    UCombatResolutionComponent();

    void RecieveHit(FAtkHitData& Hit);

    //--------------------------------
    // Hit reactions
    //--------------------------------

    UFUNCTION(BlueprintPure, Category = "Resolution")
    const FHitMontages& GetHitReactions() { return hitReactions; }

    //--------------------------------
    // Immunity
    //--------------------------------

    UFUNCTION(BlueprintCallable, Category = "Resolution")
    void SetImmunity(bool bFlag) { bImmune = bFlag; }

    UFUNCTION(BlueprintPure, Category = "Resolution")
    bool IsImmune() const { return bImmune; }

    //--------------------------------
    // Poise
    //--------------------------------

    UFUNCTION(BlueprintPure, Category = "Resolution")
    int GetPoiseBase() const { return poiseBase; }

    UFUNCTION(BlueprintCallable, Category = "Resolution")
    void SetPoiseBase(int NewPoise) { poiseBase = FMath::Max(0, NewPoise); }

    UFUNCTION(BlueprintPure, Category = "Resolution")
    int GetPoiseCalc() const { return poiseCalc; }

    UFUNCTION(BlueprintCallable, Category = "Resolution")
    void SetPoiseCalc(int NewPoise);

    UFUNCTION(BlueprintCallable, Category = "Resolution")
    void ResetPoiseCalc();
};