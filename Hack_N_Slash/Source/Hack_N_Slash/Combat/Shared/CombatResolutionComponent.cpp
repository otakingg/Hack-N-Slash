#include "CombatResolutionComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../../Interfaces/CharAnimInterface.h"
#include "../../Tags/CharacterStateTagNamespaces.h"
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

    ownerChar->LandedDelegate.AddDynamic(this, &UCombatResolutionComponent::HandleLanded);
    stateMachineComp = ownerChar->FindComponentByClass<UStateMachineComponent>();

	if (USkeletalMeshComponent* skeletalMeshComp = ownerChar->GetMesh()) iAnimInst = Cast<ICharAnimInterface>(skeletalMeshComp->GetAnimInstance());
}

void UCombatResolutionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (ownerChar) ownerChar->LandedDelegate.RemoveDynamic(this, &UCombatResolutionComponent::HandleLanded);
    Super::EndPlay(EndPlayReason);
}

void UCombatResolutionComponent::RecieveHit(FAtkHitData& Hit)
{
    //--------------------------------
    // Immune
    //--------------------------------

    if (vulnerabilityState == ECombatVulnerability::Immune) return;
    
    //--------------------------------
    // Block Gate
    //--------------------------------

    if (Hit.resolvedReaction == HitTags::BlockHit || Hit.resolvedReaction == HitTags::BlockBreak) return;

    //--------------------------------
    // Counter → open vulnerability
    //--------------------------------

    if (Hit.bIsCounterFollowUp && !IsVulnerable()) EnterVulnerable();

    //--------------------------------
    // Poise gate
    //--------------------------------

    if (!IsVulnerable() && HasHigherPoise(Hit)) return;

    //--------------------------------
    // Reaction
    //--------------------------------

    ResolveReaction(Hit);
}

void UCombatResolutionComponent::EnterVulnerable()
{
    UWorld* world = GetWorld();
    if (!world) return;

    FTimerManager& timerManager = world->GetTimerManager();
    if (timerManager.IsTimerActive(TH_Vulnerable)) timerManager.ClearTimer(TH_Vulnerable);

    vulnerabilityState = ECombatVulnerability::Vulnerable;

    timerManager.SetTimer(TH_Vulnerable, this, &UCombatResolutionComponent::ExitVulnerable, vulnerableDuration, false);
}

void UCombatResolutionComponent::ExitVulnerable()
{
    if (UWorld* world = GetWorld())
    {
        FTimerManager& timerManager = world->GetTimerManager();
        if (timerManager.IsTimerActive(TH_Vulnerable)) timerManager.ClearTimer(TH_Vulnerable);
    }
    vulnerabilityState = ECombatVulnerability::Normal;
}

bool UCombatResolutionComponent::HasHigherPoise(const FAtkHitData& Hit)
{
    UCombatResolutionComponent* atkerCmbtResComp = Hit.attacker ? Hit.attacker->FindComponentByClass<UCombatResolutionComponent>() : nullptr;
    if (!atkerCmbtResComp) return true;

    int attackerPoise = Hit.poiseOverride < 0 ? atkerCmbtResComp->GetPoise() : Hit.poiseOverride;
    attackerPoise = FMath::Clamp(attackerPoise, 0, 5);

    return attackerPoise < poise;
}

void UCombatResolutionComponent::ResolveReaction(FAtkHitData& Hit)
{
    switch (Hit.attackIntent)
    {

        case EAttackIntent::Flinch:

            if (IsVulnerable() && reactionPermissions.bAllowStagger) Hit.resolvedReaction = IsAirborne() ? HitTags::StaggerAir : HitTags::Stagger;
            else if (reactionPermissions.bAllowFlinch) Hit.resolvedReaction = HitTags::Flinch;
            break;


        case EAttackIntent::Stagger:

            if (reactionPermissions.bAllowStagger) Hit.resolvedReaction = IsAirborne() ? HitTags::StaggerAir : HitTags::Stagger;
            else if (reactionPermissions.bAllowFlinch) Hit.resolvedReaction = HitTags::Flinch;
            break;


        case EAttackIntent::Launch:

            if (reactionPermissions.bAllowLaunch) Hit.resolvedReaction = HitTags::Launch;
            else if (reactionPermissions.bAllowStagger) Hit.resolvedReaction = IsAirborne() ? HitTags::StaggerAir : HitTags::Stagger;
            else if (reactionPermissions.bAllowFlinch) Hit.resolvedReaction = HitTags::Flinch;
            break;


        case EAttackIntent::Knockback:

            if (reactionPermissions.bAllowKnockback) Hit.resolvedReaction = HitTags::Knockback;
            else if (reactionPermissions.bAllowStagger) Hit.resolvedReaction = IsAirborne() ? HitTags::StaggerAir : HitTags::Stagger;
            else if (reactionPermissions.bAllowFlinch) Hit.resolvedReaction = HitTags::Flinch;
            break;


        case EAttackIntent::Knockdown:

            if (reactionPermissions.bAllowKnockdown) Hit.resolvedReaction = HitTags::Knockdown;
            else if (reactionPermissions.bAllowStagger) Hit.resolvedReaction = IsAirborne() ? HitTags::StaggerAir : HitTags::Stagger;
            else if (reactionPermissions.bAllowFlinch) Hit.resolvedReaction = HitTags::Flinch;
            break;

    }

    //--------------------------------
    // Air juggle limiter
    //--------------------------------

    if (IsAirborne())
    {
        if (CanAirJuggle()) ++currentAirHits;
        else
        {
            Hit.motionVelocity = FVector::ZeroVector;
            Hit.resolvedReaction = HitTags::Knockdown;
        }
    }
}

bool UCombatResolutionComponent::CanAirJuggle() { return bUnlimitedJuggle || (currentAirHits < maxAirHits); }

bool UCombatResolutionComponent::IsAirborne() const
{
    return (stateMachineComp && stateMachineComp->IsAirborne() || ownerChar && ownerChar->GetCharacterMovement() && ownerChar->GetCharacterMovement()->IsFalling());
}

bool UCombatResolutionComponent::IsGrounded() const
{
    return (stateMachineComp && stateMachineComp->IsGrounded() || ownerChar && ownerChar->GetCharacterMovement() && ownerChar->GetCharacterMovement()->IsMovingOnGround());
}

void UCombatResolutionComponent::HandleLanded(const FHitResult& Hit) { currentAirHits = 0; }

FHitMontages UCombatResolutionComponent::GetHitReactions() const { return hitReactions; }

float UCombatResolutionComponent::PlayHitReaction(UAnimMontage* Montage, FName Section)
{
    float duration = 0.0f;
    if (iAnimInst) duration = iAnimInst->PlayMontageHNS(Montage, Section);
    return duration;
}