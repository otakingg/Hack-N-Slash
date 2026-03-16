#include "CombatResolutionComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../Interfaces/CombatInstigator.h"
#include "../Structs/FAtkHitData.h"
#include "../Characters/StateMachineComponent.h"

namespace ReactionTags
{
    static const FGameplayTag None = FGameplayTag::RequestGameplayTag("State.Action.None");

    static const FGameplayTag Flinch = FGameplayTag::RequestGameplayTag("State.Action.Reaction.Flinch");

    static const FGameplayTag Stagger = FGameplayTag::RequestGameplayTag("State.Action.Reaction.Stagger");

    static const FGameplayTag Launch = FGameplayTag::RequestGameplayTag("State.Action.Reaction.Launch");

    static const FGameplayTag Knockback = FGameplayTag::RequestGameplayTag("State.Action.Reaction.Knockback");

    static const FGameplayTag Knockdown = FGameplayTag::RequestGameplayTag("State.Action.Reaction.Knockdown");
}

namespace MovementTags
{
    static const FGameplayTag Airborne = FGameplayTag::RequestGameplayTag("State.Movement.Airborne");
}

UCombatResolutionComponent::UCombatResolutionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UCombatResolutionComponent::BeginPlay()
{
    Super::BeginPlay();

    ownerChar = GetOwner<ACharacter>();

    if (ownerChar)
    {
        icombatInstigator = Cast<ICombatInstigator>(ownerChar);
        ownerChar->LandedDelegate.AddDynamic(this, &UCombatResolutionComponent::HandleLanded);
        stateMachineComp = ownerChar->FindComponentByClass<UStateMachineComponent>();
    }
}

void UCombatResolutionComponent::ResolveHit(FAtkHitData& Hit)
{
    Hit.resolvedReaction = ReactionTags::None;

    //--------------------------------
    // Immune
    //--------------------------------

    if (vulnerabilityState == EVulnerabilityState::Immune) return;

    //--------------------------------
    // Counter → open vulnerability
    // Defense
    //--------------------------------

    if (Hit.bIsCounterFollowUp) EnterVulnerable();
    else if (ResolveCustomReaction(Hit)) return;
    else if (ResolveDefense(Hit)) return;

    //--------------------------------
    // Armor gate
    //--------------------------------

    if (!IsVulnerable() && HasArmorAgainst(Hit))
    {
        if (ReactionPermissions.bAllowFlinch) Hit.resolvedReaction = ReactionTags::Flinch;
        return;
    }

    //--------------------------------
    // Reaction
    //--------------------------------

    ResolveReaction(Hit);
}

bool UCombatResolutionComponent::ResolveDefense(FAtkHitData& Hit)
{
    return false;
}

void UCombatResolutionComponent::EnterVulnerable()
{
    vulnerabilityState = EVulnerabilityState::Vulnerable;
    GetWorld()->GetTimerManager().SetTimer(VulnerableTimer, this, &UCombatResolutionComponent::ExitVulnerable, VulnerableDuration, false);
}

void UCombatResolutionComponent::ExitVulnerable() { vulnerabilityState = EVulnerabilityState::Normal; }

bool UCombatResolutionComponent::IsVulnerable() const { return vulnerabilityState == EVulnerabilityState::Vulnerable; }

bool UCombatResolutionComponent::HasArmorAgainst(const FAtkHitData& Hit)
{
    if (!icombatInstigator) return false;

    ICombatInstigator* iAtkerCmbInst {Cast<ICombatInstigator>(Hit.attacker)};
    if (!iAtkerCmbInst) return true;

    int attackerPowLvl = Hit.powerLevelOverride < 0 ? iAtkerCmbInst->GetPowerLevel() + Hit.powerLevelAddition : Hit.powerLevelOverride;
    attackerPowLvl = FMath::Clamp(attackerPowLvl, 0, 3);

    return attackerPowLvl < icombatInstigator->GetPowerLevel();
}

void UCombatResolutionComponent::ResolveReaction(FAtkHitData& Hit)
{
    switch (Hit.attackIntent)
    {

        case EAttackIntent::Flinch:

            if (IsVulnerable() && ReactionPermissions.bAllowStagger) Hit.resolvedReaction = ReactionTags::Stagger;
            else if (ReactionPermissions.bAllowFlinch) Hit.resolvedReaction = ReactionTags::Flinch;
            break;


        case EAttackIntent::Stagger:

            if (ReactionPermissions.bAllowStagger) Hit.resolvedReaction = ReactionTags::Stagger;
            break;


        case EAttackIntent::Launch:

            if (ReactionPermissions.bAllowLaunch) Hit.resolvedReaction = ReactionTags::Launch;
            break;


        case EAttackIntent::Knockback:

            if (ReactionPermissions.bAllowKnockback) Hit.resolvedReaction = ReactionTags::Knockback;
            break;


        case EAttackIntent::Knockdown:

            if (ReactionPermissions.bAllowKnockdown) Hit.resolvedReaction = ReactionTags::Knockdown;
            break;

    }

    //--------------------------------
    // Air juggle limiter
    //--------------------------------

    if (IsAirborne())
    {
        if (CanAirJuggle()) ++CurrentAirHits;
        else
        {
            Hit.motionVelocity = FVector::ZeroVector;
            Hit.resolvedReaction = ReactionTags::Knockdown;
        }
    }
}

bool UCombatResolutionComponent::CanAirJuggle() { return bUnlimitedJuggle || (CurrentAirHits < MaxAirHits); }

bool UCombatResolutionComponent::IsAirborne() const
{
    return (stateMachineComp && stateMachineComp->IsInMovementTag(MovementTags::Airborne)) || ownerChar && ownerChar->GetCharacterMovement() && ownerChar->GetCharacterMovement()->IsFalling();
}

void UCombatResolutionComponent::HandleLanded(const FHitResult& Hit) { CurrentAirHits = 0; }