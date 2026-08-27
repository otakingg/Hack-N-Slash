#include "CombatResolutionComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../../Interfaces/CombatInstigator.h"
#include "../../Structs/FAtkHitData.h"
#include "../..//Utility/Tags.h"

UCombatResolutionComponent::UCombatResolutionComponent() { PrimaryComponentTick.bCanEverTick = false; }

void UCombatResolutionComponent::BeginPlay()
{
    Super::BeginPlay();

    if (!EnsureReferences()) return;

    ownerChar->LandedDelegate.AddDynamic(this, &UCombatResolutionComponent::HandleLanded);
}

void UCombatResolutionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (ownerChar) ownerChar->LandedDelegate.RemoveDynamic(this, &UCombatResolutionComponent::HandleLanded);
    Super::EndPlay(EndPlayReason);
}

bool UCombatResolutionComponent::EnsureReferences()
{
    if (!ownerChar) ownerChar = GetOwner<ACharacter>();
    if (!ownerChar)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UCombatResolutionComponent] Owner is not an ACharacter: %s"), *GetNameSafe(GetOwner()));
        return false;
    }

	if (!iCmbtInst) iCmbtInst = Cast<ICombatInstigator>(ownerChar);
	if (!iCmbtInst)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UCombatResolutionComponent] Owner does not implement ICombatInstigator: %s"), *GetNameSafe(ownerChar));
		return false;
	}

    return true;
}

void UCombatResolutionComponent::RecieveHit(FAtkHitData& Hit)
{
    //--------------------------------
    // Immune
    //--------------------------------

    if (vulnerabilityState == ECombatVulnerability::Immune) return;
    
    //--------------------------------
    // Reaction Gate
    //--------------------------------

    if (!Hit.resolvedReaction.MatchesTag(Tags::StateMachine::Action::None)) return; // Already has a reaction, so leave

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

bool UCombatResolutionComponent::HasHigherPoise(const FAtkHitData& Hit) { return Hit.poise < poise; }

void UCombatResolutionComponent::ResolveReaction(FAtkHitData& Hit)
{
    switch (Hit.attackIntent)
    {

        case EAttackIntent::Flinch:

            if (IsAirborne()) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Air;
            else if (IsVulnerable() && reactionPermissions.bAllowStagger) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Stagger;
            else if (reactionPermissions.bAllowFlinch) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Flinch;
            break;


        case EAttackIntent::Stagger:

            if (IsAirborne()) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Air;
            else if (reactionPermissions.bAllowStagger) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Stagger;
            else if (reactionPermissions.bAllowFlinch) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Flinch;
            break;


        case EAttackIntent::Launch:

            if (reactionPermissions.bAllowLaunch) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Launch;
            else if (IsAirborne()) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Air;
            else if (reactionPermissions.bAllowStagger) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Stagger;
            else if (reactionPermissions.bAllowFlinch) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Flinch;
            break;


        case EAttackIntent::Knockback:

            if (reactionPermissions.bAllowKnockback) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Knockback;
            else if (IsAirborne()) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Air;
            else if (reactionPermissions.bAllowStagger) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Stagger;
            else if (reactionPermissions.bAllowFlinch) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Flinch;
            break;


        case EAttackIntent::Knockdown:

            if (reactionPermissions.bAllowKnockdown) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Knockdown;
            else if (IsAirborne()) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Air;
            else if (reactionPermissions.bAllowStagger) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Stagger;
            else if (reactionPermissions.bAllowFlinch) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Flinch;
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
            Hit.distance = 0.0f;
            Hit.resolvedReaction = Tags::StateMachine::Action::None;
        }
    }
}

bool UCombatResolutionComponent::CanAirJuggle() { return bUnlimitedJuggle || (currentAirHits < maxAirHits); }

bool UCombatResolutionComponent::IsAirborne() const
{
    if (iCmbtInst) return iCmbtInst->IsAirborne();
    else return ownerChar && ownerChar->GetCharacterMovement() && ownerChar->GetCharacterMovement()->IsFalling();
}

bool UCombatResolutionComponent::IsGrounded() const
{
    if (iCmbtInst) return iCmbtInst->IsGrounded();
    else return ownerChar && ownerChar->GetCharacterMovement() && ownerChar->GetCharacterMovement()->IsMovingOnGround();
}

void UCombatResolutionComponent::HandleLanded(const FHitResult& Hit) { currentAirHits = 0; }

FHitMontages UCombatResolutionComponent::GetHitReactions() const { return hitReactions; }