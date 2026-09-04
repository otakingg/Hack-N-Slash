#include "CombatResolutionComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../../Structs/FAtkHitData.h"
#include "../../Characters/Shared/StateMachineComponent.h"
#include "../..//Utility/Tags.h"

UCombatResolutionComponent::UCombatResolutionComponent() { PrimaryComponentTick.bCanEverTick = false; }

void UCombatResolutionComponent::BeginPlay()
{
    Super::BeginPlay();

    if (!EnsureReferences()) return;

    poiseCalc = poiseBase;
    ownerChar->LandedDelegate.AddDynamic(this, &UCombatResolutionComponent::HandleLanded); // Bind to the character's landed event for air juggle control
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

    if (!moveComp) moveComp = ownerChar->GetCharacterMovement();
    if (!moveComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UCombatResolutionComponent] Owner doesn't have a valid Movement Comp: %s"), *GetNameSafe(GetOwner()));
        return false;
    }

    if (!stateMachineComp) stateMachineComp = ownerChar->GetComponentByClass<UStateMachineComponent>();

    return true;
}

void UCombatResolutionComponent::RecieveHit(FAtkHitData& Hit)
{    
    //--------------------------------
    // Reaction Gate
    //--------------------------------

    if (!Hit.resolvedReaction.MatchesTag(Tags::StateMachine::Action::None)) return; // Reaction already chosen, so leave

    //--------------------------------
    // Immunity Gate
    //--------------------------------

    if (bImmune) // Immune to reactions
    {
        Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::NoReact;
        return;
    }

    //--------------------------------
    // Poise gate
    //--------------------------------

    // This combatent has higher poise than the incoming attack, so don't react if not airborne
    // Allowing reactions when airborne as a design decision
    if (HasHigherPoise(Hit) && !IsAirborne())
    {
        Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::NoReact;
        return;
    }

    //--------------------------------
    // Reaction
    //--------------------------------

    ResolveReaction(Hit); // Choose the correct reaction to play
}

void UCombatResolutionComponent::ResolveReaction(FAtkHitData& Hit)
{
    switch (Hit.attackIntent)
    {

        case EAttackIntent::Flinch:
            if (IsAirborne()) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Air;
            else if (reactionPermissions.bAllowFlinch) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Flinch;
            else Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::NoReact;
            break;


        case EAttackIntent::Stagger:
            if (IsAirborne()) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Air;
            else if (reactionPermissions.bAllowStagger) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Stagger;
            else if (reactionPermissions.bAllowFlinch) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Flinch;
            else Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::NoReact;
            break;


        case EAttackIntent::Launch:
            if (reactionPermissions.bAllowLaunch) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Launch;
            else if (IsAirborne()) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Air;
            else if (reactionPermissions.bAllowStagger) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Stagger;
            else if (reactionPermissions.bAllowFlinch) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Flinch;
            else Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::NoReact;
            break;


        case EAttackIntent::Knockback:
            if (reactionPermissions.bAllowKnockback) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Knockback;
            else if (IsAirborne()) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Air;
            else if (reactionPermissions.bAllowStagger) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Stagger;
            else if (reactionPermissions.bAllowFlinch) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Flinch;
            else Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::NoReact;
            break;


        case EAttackIntent::Knockdown:
            if (reactionPermissions.bAllowKnockdown) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Knockdown;
            else if (IsAirborne()) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Air;
            else if (reactionPermissions.bAllowStagger) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Stagger;
            else if (reactionPermissions.bAllowFlinch) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Flinch;
            else Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::NoReact;
            break;

        case EAttackIntent::BounceGround:
            if (reactionPermissions.bAllowBounceGround) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::BounceGround;
            else if (reactionPermissions.bAllowKnockdown) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Knockdown;
            else if (IsAirborne()) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Air;
            else if (reactionPermissions.bAllowStagger) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Stagger;
            else if (reactionPermissions.bAllowFlinch) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Flinch;
            else Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::NoReact;
            break;

        case EAttackIntent::BounceWall:
            if (reactionPermissions.bAllowWallSplat) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::BounceWall;
            else Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::NoReact;
            break;

        default:
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

bool UCombatResolutionComponent::IsAirborne() const
{
    if (stateMachineComp) return stateMachineComp->IsAirborne();
    else return moveComp && moveComp->IsFalling();
}

bool UCombatResolutionComponent::IsGrounded() const
{
    if (stateMachineComp) return stateMachineComp->IsGrounded();
    else return moveComp && moveComp->IsMovingOnGround();
}

bool UCombatResolutionComponent::HasHigherPoise(const FAtkHitData& Hit) const { return Hit.poise < poiseCalc; }

void UCombatResolutionComponent::SetPoiseCalc(int NewPoise)
{
    if (bPoiseOverriden) return; // If poise is currently overriden, return
    else // Set the new poise and mark it as overriden
    {
        poiseCalc = FMath::Max(-1, NewPoise);
        bPoiseOverriden = true;
    }
}

void UCombatResolutionComponent::ResetPoiseCalc()
{
    poiseCalc = poiseBase; // Reset calced poise to base poise
    bPoiseOverriden = false; // Poise is no longer overriden
}