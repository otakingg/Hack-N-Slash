#include "CombatResolutionComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../../Interfaces/CharAnimInterface.h"
#include "../../Tags/CharacterStateTagNamespaces.h"
#include "../../Interfaces/CombatInstigator.h"
#include "../../Structs/FAtkHitData.h"
#include "../../Characters/Shared/StateMachineComponent.h"

UCombatResolutionComponent::UCombatResolutionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UCombatResolutionComponent::BeginPlay()
{
    Super::BeginPlay();

    ownerChar = GetOwner<ACharacter>();
    if (!ownerChar) return;

    iCombatInstigator = Cast<ICombatInstigator>(ownerChar);
    ownerChar->LandedDelegate.AddDynamic(this, &UCombatResolutionComponent::HandleLanded);
    stateMachineComp = ownerChar->FindComponentByClass<UStateMachineComponent>();

	if (USkeletalMeshComponent* skeletalMeshComp = ownerChar->GetMesh()) iAnimInst = Cast<ICharAnimInterface>(skeletalMeshComp->GetAnimInstance());
}

void UCombatResolutionComponent::ResolveHit(FAtkHitData& Hit)
{
    Hit.resolvedReaction = ActionTags::None;

    //--------------------------------
    // Immune
    //--------------------------------

    if (vulnerabilityState == EVulnerabilityState::Immune) return;

    //--------------------------------
    // Counter → open vulnerability
    // Defense
    //--------------------------------

    if (Hit.bIsCounterFollowUp) EnterVulnerable();
    else if (ResolveDefense(Hit)) return;

    //--------------------------------
    // Armor gate
    //--------------------------------

    if (!IsVulnerable() && HasHigherPowerLvl(Hit))
    {
        if (ReactionPermissions.bAllowFlinch) Hit.resolvedReaction = HitTags::Flinch;
        return;
    }

    //--------------------------------
    // Reaction
    //--------------------------------

    ResolveReaction(Hit);
}

bool UCombatResolutionComponent::ResolveDefense(FAtkHitData& Hit)
{
    return stateMachineComp && (stateMachineComp->HasExactActiveTag(CombatTags::Parry) || stateMachineComp->HasExactActiveTag(CombatTags::Block));
}

void UCombatResolutionComponent::EnterVulnerable()
{
    vulnerabilityState = EVulnerabilityState::Vulnerable;
    GetWorld()->GetTimerManager().SetTimer(VulnerableTimer, this, &UCombatResolutionComponent::ExitVulnerable, VulnerableDuration, false);
}

void UCombatResolutionComponent::ExitVulnerable() { vulnerabilityState = EVulnerabilityState::Normal; }

bool UCombatResolutionComponent::IsVulnerable() const { return vulnerabilityState == EVulnerabilityState::Vulnerable; }

bool UCombatResolutionComponent::HasHigherPowerLvl(const FAtkHitData& Hit)
{
    if (!iCombatInstigator) return false;

    ICombatInstigator* iAtkerCmbInst = Cast<ICombatInstigator>(Hit.attacker);
    if (!iAtkerCmbInst) return true;

    int attackerPowLvl = Hit.powerLevelOverride < 0 ? iAtkerCmbInst->GetPowerLevel() + Hit.powerLevelAddition : Hit.powerLevelOverride;
    attackerPowLvl = FMath::Clamp(attackerPowLvl, 0, iAtkerCmbInst->GetPowerLevelMax());

    return attackerPowLvl < powerLvl;
}

void UCombatResolutionComponent::ResolveReaction(FAtkHitData& Hit)
{
    switch (Hit.attackIntent)
    {

        case EAttackIntent::Flinch:

            if (IsVulnerable() && ReactionPermissions.bAllowStagger) Hit.resolvedReaction = IsAirborne() ? HitTags::StaggerAir : HitTags::Stagger;
            else if (ReactionPermissions.bAllowFlinch) Hit.resolvedReaction = HitTags::Flinch;
            break;


        case EAttackIntent::Stagger:

            if (ReactionPermissions.bAllowStagger) Hit.resolvedReaction = IsAirborne() ? HitTags::StaggerAir : HitTags::Stagger;
            break;


        case EAttackIntent::Launch:

            if (ReactionPermissions.bAllowLaunch) Hit.resolvedReaction = HitTags::Launch;
            break;


        case EAttackIntent::Knockback:

            if (ReactionPermissions.bAllowKnockback) Hit.resolvedReaction = HitTags::Knockback;
            break;


        case EAttackIntent::Knockdown:

            if (ReactionPermissions.bAllowKnockdown) Hit.resolvedReaction = HitTags::Knockdown;
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
            Hit.resolvedReaction = HitTags::Knockdown;
        }
    }
}

bool UCombatResolutionComponent::CanAirJuggle() { return bUnlimitedJuggle || (CurrentAirHits < MaxAirHits); }

bool UCombatResolutionComponent::IsAirborne() const
{
    return (stateMachineComp && stateMachineComp->IsAirborne() || ownerChar && ownerChar->GetCharacterMovement() && ownerChar->GetCharacterMovement()->IsFalling());
}

bool UCombatResolutionComponent::IsGrounded() const
{
    return (stateMachineComp && stateMachineComp->IsGrounded() || ownerChar && ownerChar->GetCharacterMovement() && ownerChar->GetCharacterMovement()->IsMovingOnGround());
}

void UCombatResolutionComponent::HandleLanded(const FHitResult& Hit) { CurrentAirHits = 0; }

FHitMontages UCombatResolutionComponent::GetHitReactions() const { return hitReactions; }

float UCombatResolutionComponent::PlayHitReaction(UAnimMontage* Montage, FName Section)
{
    float duration = 0.0f;
    
    if (iAnimInst) duration = iAnimInst->PlayMontageHNS(Montage, Section);

    return duration;
}