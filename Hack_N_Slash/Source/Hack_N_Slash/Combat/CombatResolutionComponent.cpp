#include "CombatResolutionComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../Characters/StateMachineComponent.h"

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
        ownerChar->LandedDelegate.AddDynamic(this, &UCombatResolutionComponent::ResetAirState);
        stateMachineComp = ownerChar->FindComponentByClass<UStateMachineComponent>();
    }
}

void UCombatResolutionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    airTime += DeltaTime;
    if (airTime >= maxAirTime)
    {
        Fall();
        PrimaryComponentTick.bCanEverTick = false;
    }
}

void UCombatResolutionComponent::ResolveHit(FAtkHitData& HitData)
{
    HitData.resolvedReaction = FGameplayTag::RequestGameplayTag(FName("State.Action.None"));
    if (bAirTimerExpired) HitData.knockbackVelocity = FVector::ZeroVector; // Force this actor to fall

    //----------------------------------
    // Immune check
    //----------------------------------

    if (reactionState == EReactionState::Immune) return;

    //----------------------------------
    // Defense check
    //----------------------------------

    if (Step_Parry(HitData)) return;
    if (Step_Block(HitData)) return;

    //----------------------------------
    // Parry / Perfect Block Follow-up
    //----------------------------------

    if (HitData.bIsCounter)
    {
        postureCurrent = 0.0f;
        EnterVulnerable();
    }

    //----------------------------------
    // Armor
    //----------------------------------

    if (!IsVulnerable() && Step_Armor(HitData)) return;

    //----------------------------------
    // Posture
    //----------------------------------

    if (bUsePosture) Step_Posture(HitData);

    //----------------------------------
    // Reaction
    //----------------------------------

    Step_Reaction(HitData);
}

// Got Parried
bool UCombatResolutionComponent::Step_Parry(FAtkHitData& HitData)
{
    // If got parried: HitData.resolvedReaction = TAG_State_Action_Reaction_Parry; return true;
    return false;
}

// Blocked
bool UCombatResolutionComponent::Step_Block(FAtkHitData& HitData)
{
    // If perfect blocked trigger perfect blocked event; return true;
    // If blocked: HitData.resolvedReaction = TAG_State_Action_Reaction_Block_Hit; return true;
    // If block Broken: TAG_State_Action_Reaction_Block_Broken; return true;
    return false;
}

bool UCombatResolutionComponent::Step_Armor(FAtkHitData& HitData)
{
    if (HitData.atkType < armorType)
    {
        HitData.resolvedReaction = FGameplayTag::RequestGameplayTag(FName("State.Action.None"));
        return true;
    }
    else return false;
}

void UCombatResolutionComponent::Step_Posture(FAtkHitData& HitData)
{
    if (IsVulnerable()) return;

    float postureDmg = HitData.dmgPosture;

    postureCurrent -= postureDmg;
    postureCurrent = FMath::Clamp(postureCurrent, 0.f, postureMax);

    if (postureCurrent <= 0.0f) EnterVulnerable();
}

void UCombatResolutionComponent::Step_Reaction(FAtkHitData& HitData)
{
    if (!IsVulnerable())
    {
        HitData.resolvedReaction = FGameplayTag::RequestGameplayTag(FName("State.Action.Reaction.Flinch"));
        return;
    }

    if (IsAirborne() && airTime <= 0.0f) PrimaryComponentTick.bCanEverTick = true;

    switch (HitData.reactionType)
    {
        case EAtkReactionType::Launch:

            if (!bLaunchImmune) HitData.resolvedReaction = FGameplayTag::RequestGameplayTag(FName("State.Action.Reaction.Launch"));
            break;

        case EAtkReactionType::Knockdown:

            HitData.resolvedReaction = FGameplayTag::RequestGameplayTag(FName("State.Action.Reaction.Knockdown"));
            return;

        case EAtkReactionType::Knockback:

            HitData.resolvedReaction = FGameplayTag::RequestGameplayTag(FName("State.Action.Reaction.Knockback"));
            return;

        default:

            HitData.resolvedReaction = FGameplayTag::RequestGameplayTag(FName("State.Action.Reaction.Stagger"));
            break;
    }
}

void UCombatResolutionComponent::EnterVulnerable()
{
    reactionState = EReactionState::Vulnerable;
    GetWorld()->GetTimerManager().SetTimer(TH_Vulnerable, this, &UCombatResolutionComponent::ExitVulnerable, vulnerableDuration, false);
}

void UCombatResolutionComponent::ExitVulnerable()
{
    reactionState = EReactionState::Normal;
    postureCurrent = postureMax;
}

bool UCombatResolutionComponent::IsVulnerable() const { return reactionState == EReactionState::Vulnerable; }

void UCombatResolutionComponent::Fall()
{
    bAirTimerExpired = true;
    if (UActionState* state = stateMachineComp->GetActionStateByTag(FGameplayTag::RequestGameplayTag(FName("State.Action.Reaction.Knockdown")))) stateMachineComp->ChangeActionState(state, true);
}

//bool UCombatResolutionComponent::CanBeLaunched() const { return !bLaunchImmune; }

void UCombatResolutionComponent::ResetAirState(const FHitResult& Hit)
{
    airTime = 0.0f;
    bAirTimerExpired = false;
}

bool UCombatResolutionComponent::IsAirborne() const
{
    return (stateMachineComp && stateMachineComp->IsInMovementTag(FGameplayTag::RequestGameplayTag(FName("State.Movement.Airborne")))) || ownerChar && ownerChar->GetCharacterMovement() && ownerChar->GetCharacterMovement()->IsFalling();
}
