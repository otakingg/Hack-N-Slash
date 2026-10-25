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

    if (Hit.resolvedReaction != EHitReaction::None) return; // Reaction already chosen, so leave

    //--------------------------------
    // Immunity Gate
    //--------------------------------

    if (bImmune) // Immune to reactions
    {
        Hit.resolvedReaction = EHitReaction::NoReact;
        return;
    }

    //--------------------------------
    // Poise gate
    //--------------------------------

    // This combatent has higher poise than the incoming attack, so don't react if not airborne
    // Allowing reactions when airborne as a design decision
    if (HasHigherPoise(Hit) && !IsAirborne())
    {
        Hit.resolvedReaction = EHitReaction::NoReact;
        return;
    }

    //--------------------------------
    // Reaction
    //--------------------------------

    ResolveReaction(Hit); // Choose the correct reaction to play
}

void UCombatResolutionComponent::ResolveReaction(FAtkHitData& Hit)
{
    switch(Hit.attackReaction)
    {
        case EHitReaction::BounceGround:
            if (reactionPermissions.bAllowBounceGround && hitReactions.bounceGround)      Hit.resolvedReaction = EHitReaction::BounceGround;
            else if (reactionPermissions.bAllowLaunchDown && hitReactions.launchDown)     Hit.resolvedReaction = EHitReaction::LaunchDown;
            else if (IsAirborne() && hitReactions.air)                                    Hit.resolvedReaction = EHitReaction::Air;
            else if (reactionPermissions.bAllowStagger && hitReactions.staggerDownFront2) Hit.resolvedReaction = EHitReaction::StaggerDownFront2;
            else                                                                          Hit.resolvedReaction = EHitReaction::NoReact;
            break;

        case EHitReaction::LaunchDown:
            if (reactionPermissions.bAllowLaunchDown && hitReactions.launchDown)          Hit.resolvedReaction = EHitReaction::LaunchDown;
            else if (IsAirborne() && hitReactions.air)                                    Hit.resolvedReaction = EHitReaction::Air;
            else if (reactionPermissions.bAllowStagger && hitReactions.staggerDownFront2) Hit.resolvedReaction = EHitReaction::StaggerDownFront2;
            else                                                                          Hit.resolvedReaction = EHitReaction::NoReact;
            break;

        case EHitReaction::LaunchFwdHard:
            if (reactionPermissions.bAllowLaunchFwdHard && hitReactions.launchFwdHard)           Hit.resolvedReaction = EHitReaction::LaunchFwdHard;
            else if (reactionPermissions.bAllowLaunchFwdSoft && hitReactions.launchFwdSoftFront) Hit.resolvedReaction = EHitReaction::LaunchFwdSoftFront;
            else if (IsAirborne() && hitReactions.air)                                           Hit.resolvedReaction = EHitReaction::Air;
            else if (reactionPermissions.bAllowStagger && hitReactions.staggerUpFront2)          Hit.resolvedReaction = EHitReaction::StaggerUpFront2;
            else                                                                                 Hit.resolvedReaction = EHitReaction::NoReact;
            break;

        case EHitReaction::LaunchFwdSoftBack:
            if (reactionPermissions.bAllowLaunchFwdSoft && hitReactions.launchFwdSoftBack) Hit.resolvedReaction = EHitReaction::LaunchFwdSoftBack;
            else if (IsAirborne() && hitReactions.air)                                     Hit.resolvedReaction = EHitReaction::Air;
            else if (reactionPermissions.bAllowStagger && hitReactions.staggerBack)        Hit.resolvedReaction = EHitReaction::StaggerBack;
            else                                                                           Hit.resolvedReaction = EHitReaction::NoReact;
            break;

        case EHitReaction::LaunchFwdSoftFront:
            if (reactionPermissions.bAllowLaunchFwdSoft && hitReactions.launchFwdSoftFront) Hit.resolvedReaction = EHitReaction::LaunchFwdSoftFront;
            else if (IsAirborne() && hitReactions.air)                                      Hit.resolvedReaction = EHitReaction::Air;
            else if (reactionPermissions.bAllowStagger && hitReactions.staggerUpFront2)     Hit.resolvedReaction = EHitReaction::StaggerUpFront2;
            else                                                                            Hit.resolvedReaction = EHitReaction::NoReact;
            break;

        case EHitReaction::LaunchUp:
            if (reactionPermissions.bAllowLaunchUp && hitReactions.launchUp)            Hit.resolvedReaction = EHitReaction::LaunchUp;
            else if (IsAirborne())                                                      Hit.resolvedReaction = EHitReaction::Air;
            else if (reactionPermissions.bAllowStagger && hitReactions.staggerUpFront2) Hit.resolvedReaction = EHitReaction::StaggerUpFront2;
            else                                                                        Hit.resolvedReaction = EHitReaction::NoReact;
            break;
        
        case EHitReaction::StaggerBack:
            if (IsAirborne() && hitReactions.air)                                   Hit.resolvedReaction = EHitReaction::Air;
            else if (reactionPermissions.bAllowStagger && hitReactions.staggerBack) Hit.resolvedReaction = EHitReaction::StaggerBack;
            else                                                                    Hit.resolvedReaction = EHitReaction::NoReact;
            break;

        case EHitReaction::StaggerDownFront1:
            if (IsAirborne() && hitReactions.air)                                         Hit.resolvedReaction = EHitReaction::Air;
            else if (reactionPermissions.bAllowStagger && hitReactions.staggerDownFront1) Hit.resolvedReaction = EHitReaction::StaggerDownFront1;
            else                                                                          Hit.resolvedReaction = EHitReaction::NoReact;
            break;

        case EHitReaction::StaggerDownFront2:
            if (IsAirborne() && hitReactions.air)                                         Hit.resolvedReaction = EHitReaction::Air;
            else if (reactionPermissions.bAllowStagger && hitReactions.staggerDownFront2) Hit.resolvedReaction = EHitReaction::StaggerDownFront2;
            else                                                                          Hit.resolvedReaction = EHitReaction::NoReact;
            break;

        case EHitReaction::StaggerDownFront3:
            if (IsAirborne() && hitReactions.air)                                         Hit.resolvedReaction = EHitReaction::Air;
            else if (reactionPermissions.bAllowStagger && hitReactions.staggerDownFront3) Hit.resolvedReaction = EHitReaction::StaggerDownFront3;
            else                                                                          Hit.resolvedReaction = EHitReaction::NoReact;
            break;

        case EHitReaction::StaggerUpFront1:
            if (IsAirborne() && hitReactions.air)                                       Hit.resolvedReaction = EHitReaction::Air;
            else if (reactionPermissions.bAllowStagger && hitReactions.staggerUpFront1) Hit.resolvedReaction = EHitReaction::StaggerUpFront1;
            else                                                                        Hit.resolvedReaction = EHitReaction::NoReact;
            break;

        case EHitReaction::StaggerUpFront2:
            if (IsAirborne() && hitReactions.air)                                       Hit.resolvedReaction = EHitReaction::Air;
            else if (reactionPermissions.bAllowStagger && hitReactions.staggerUpFront2) Hit.resolvedReaction = EHitReaction::StaggerUpFront2;
            else                                                                        Hit.resolvedReaction = EHitReaction::NoReact;
            break;

        case EHitReaction::StaggerUpFront3:
            if (IsAirborne() && hitReactions.air)                                       Hit.resolvedReaction = EHitReaction::Air;
            else if (reactionPermissions.bAllowStagger && hitReactions.staggerUpFront3) Hit.resolvedReaction = EHitReaction::StaggerUpFront3;
            else                                                                        Hit.resolvedReaction = EHitReaction::NoReact;
            break;

        default:
            break;
    }
    
    //--------------------------------
    // Air juggle limiter
    //--------------------------------

    if (Hit.resolvedReaction == EHitReaction::Air)
    {
        if (CanAirJuggle()) ++currentAirHits;
        else
        {
            Hit.distance = 0.0f;
            Hit.resolvedReaction = EHitReaction::NoReact;
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