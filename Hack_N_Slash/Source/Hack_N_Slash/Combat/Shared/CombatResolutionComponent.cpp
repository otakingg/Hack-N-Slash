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
    if (Hit.attackIntent == Tags::StateMachine::Action::Reaction::StaggerBack)
    {
        if (IsAirborne())                           Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Air;
        else if (reactionPermissions.bAllowStagger) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::StaggerBack;
        else                                        Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::NoReact;
    }
    else if (Hit.attackIntent == Tags::StateMachine::Action::Reaction::StaggerDownFront1)
    {
        if (IsAirborne())                           Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Air;
        else if (reactionPermissions.bAllowStagger) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::StaggerDownFront1;
        else                                        Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::NoReact;
    }
    else if (Hit.attackIntent == Tags::StateMachine::Action::Reaction::StaggerDownFront2)
    {
        if (IsAirborne())                           Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Air;
        else if (reactionPermissions.bAllowStagger) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::StaggerDownFront2;
        else                                        Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::NoReact;
    }
    else if (Hit.attackIntent == Tags::StateMachine::Action::Reaction::StaggerDownFront3)
    {
        if (IsAirborne())                           Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Air;
        else if (reactionPermissions.bAllowStagger) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::StaggerDownFront3;
        else                                        Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::NoReact;
    }
    else if (Hit.attackIntent == Tags::StateMachine::Action::Reaction::StaggerUpFront1)
    {
        if (IsAirborne())                           Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Air;
        else if (reactionPermissions.bAllowStagger) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::StaggerUpFront1;
        else                                        Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::NoReact;
    }
    else if (Hit.attackIntent == Tags::StateMachine::Action::Reaction::StaggerUpFront2)
    {
        if (IsAirborne())                           Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Air;
        else if (reactionPermissions.bAllowStagger) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::StaggerUpFront2;
        else                                        Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::NoReact;
    }
    else if (Hit.attackIntent == Tags::StateMachine::Action::Reaction::StaggerUpFront3)
    {
        if (IsAirborne())                           Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Air;
        else if (reactionPermissions.bAllowStagger) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::StaggerUpFront3;
        else                                        Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::NoReact;
    }
    else if (Hit.attackIntent == Tags::StateMachine::Action::Reaction::LaunchDown)
    {
        if (reactionPermissions.bAllowLaunchDown)   Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::LaunchDown;
        else if (IsAirborne())                      Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Air;
        else if (reactionPermissions.bAllowStagger) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::StaggerDownFront2;
        else                                        Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::NoReact;
    }
    else if (Hit.attackIntent == Tags::StateMachine::Action::Reaction::LaunchFwdHard)
    {
        if (reactionPermissions.bAllowLaunchFwdHard)      Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::LaunchFwdHard;
        else if (reactionPermissions.bAllowLaunchFwdSoft) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::LaunchFwdSoftFront;
        else if (IsAirborne())                            Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Air;
        else if (reactionPermissions.bAllowStagger)       Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::StaggerUpFront2;
        else                                              Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::NoReact;
    }
    else if (Hit.attackIntent == Tags::StateMachine::Action::Reaction::LaunchFwdSoftBack)
    {
        if (reactionPermissions.bAllowLaunchFwdSoft) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::LaunchFwdSoftBack;
        else if (IsAirborne())                       Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Air;
        else if (reactionPermissions.bAllowStagger)  Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::StaggerBack;
        else                                         Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::NoReact;
    }

    else if (Hit.attackIntent == Tags::StateMachine::Action::Reaction::LaunchFwdSoftFront)
    {
        if (reactionPermissions.bAllowLaunchFwdSoft) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::LaunchFwdSoftFront;
        else if (IsAirborne())                       Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Air;
        else if (reactionPermissions.bAllowStagger)  Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::StaggerUpFront2;
        else                                         Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::NoReact;
    }
    else if (Hit.attackIntent == Tags::StateMachine::Action::Reaction::LaunchUp)
    {
        if (reactionPermissions.bAllowLaunchUp)     Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::LaunchUp;
        else if (IsAirborne())                      Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Air;
        else if (reactionPermissions.bAllowStagger) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::StaggerUpFront2;
        else                                        Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::NoReact;
    }
    else if (Hit.attackIntent == Tags::StateMachine::Action::Reaction::BounceGround)
    {
        if (reactionPermissions.bAllowBounceGround)    Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::BounceGround;
        else if (reactionPermissions.bAllowLaunchDown) Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::LaunchDown;
        else if (IsAirborne())                         Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::Air;
        else if (reactionPermissions.bAllowStagger)    Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::StaggerDownFront2;
        else                                           Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::NoReact;
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
            Hit.resolvedReaction = Tags::StateMachine::Action::Reaction::NoReact;
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

bool UCombatResolutionComponent::HasHigherPoise(const FAtkHitData& Hit) const { return poiseCalc > Hit.poise; }

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