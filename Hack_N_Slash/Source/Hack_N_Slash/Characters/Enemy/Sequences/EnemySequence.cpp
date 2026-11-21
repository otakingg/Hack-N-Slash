#include "EnemySequence.h"
#include "../../../Interfaces/Combatant.h"
#include "../EnemyBrainComponent.h"
#include "../../../Controllers/EnemyController.h"
#include "../../Shared/StateMachineComponent.h"
#include "../../../Utility/Tags.h"

void UEnemySequence::Initialize_Implementation(UEnemyBrainComponent* InBrain)
{
    brain = InBrain;

    if (cooldownMax < cooldownMin) cooldownMax = cooldownMin;

    if (bStartOnCooldown) bOnCooldown = true;
}

bool UEnemySequence::IsActive() const { return brain && brain->GetActiveSequence() == this; }

bool UEnemySequence::NoActiveSeqOrCanInterruptProactive(bool bAcceptCanInterrupt) const
{
    if (!brain) return false;

    UEnemySequence* sequence = brain->GetActiveSequence();
    return !sequence || (bAcceptCanInterrupt && sequence->IsInterruptibleProactive());
}

bool UEnemySequence::NoActiveSeqOrCanInterruptReactive(bool bAcceptCanInterrupt) const
{
    if (!brain) return false;

    UEnemySequence* sequence = brain->GetActiveSequence();
    return !sequence || (bAcceptCanInterrupt && sequence->IsInterruptibleReactive());
}

void UEnemySequence::TryStartOnCooldownTimer()
{
    if (!brain || !bStartOnCooldown || cooldownMax <= 0) return;

    if (UWorld* world = GetWorld())
    {
        cooldown = FMath::RandRange(cooldownMin, cooldownMax);
        world->GetTimerManager().SetTimer(TH_Cooldown, this, &UEnemySequence::EndCooldown, cooldown, false);
    }
}

void UEnemySequence::Finish_Implementation()
{
    if (!brain || !IsActive()) return;

    UWorld* world = GetWorld();

    sequenceIndex = 1; // Reset sequence index to 1
    bInterruptibleProactive = false; // Reset interruptibility
    bInterruptibleReactive = false; // Reset interruptibility

    if (cooldownMin == cooldownMax) cooldown = cooldownMax;
    else cooldown = FMath::RandRange(cooldownMin, cooldownMax);

    if (cooldown > 0 && world) // Enter cooldown if the sequence has a cooldown
    {
        bOnCooldown = true;
        FTimerManager& timerManager = world->GetTimerManager();
        timerManager.ClearAllTimersForObject(this);
        timerManager.SetTimer(TH_Cooldown, this, &UEnemySequence::EndCooldown, cooldown, false);
    }
    
    if (world) lastSequenceTime = world->GetTimeSeconds(); // Set last sequence time
    brain->RemoveActiveSequence(); // Sequence is no longer active

    if (AEnemyController* controller = brain->GetEnemyController()) controller->ClearFocusHNS();
}

void UEnemySequence::Abort_Implementation()
{
    if (!brain) return;

    UWorld* world = GetWorld();

    sequenceIndex = 1;
    bInterruptibleProactive = false;
    bInterruptibleReactive = false;

    if (AEnemyController* controller = brain->GetEnemyController())
    {
        controller->StopMovement();
        controller->ClearFocusHNS();
    }

    if (UStateMachineComponent* smComp = brain->GetStateMachine())
    {
        if (!HasAnyTags({Tags::StateMachine::Action::Reaction::Hit, Tags::StateMachine::Action::Reaction::Dead})) smComp->ClearActionState();
    }

    if (cooldownMin == cooldownMax) cooldown = cooldownMax;
    else cooldown = FMath::RandRange(cooldownMin, cooldownMax);

    if (cooldown > 0 && world)
    {
        bOnCooldown = true;
        FTimerManager& timerManager = world->GetTimerManager();
        timerManager.ClearAllTimersForObject(this);
        timerManager.SetTimer(TH_Cooldown, this, &UEnemySequence::EndCooldown, cooldown, false);
    }

    if (world) lastSequenceTime = world->GetTimeSeconds();
    if (IsActive()) brain->RemoveActiveSequence();
}

void UEnemySequence::HandleAnimNotify_Implementation(const FGameplayTag& NotifyTag)
{
    if (!IsActive()) return;
    else if (NotifyTag.MatchesTag(Tags::NotifyEvent::EnemyBrain::AdvanceSequence)) AdvanceSequence();
    else if (NotifyTag.MatchesTag(Tags::NotifyEvent::EnemyBrain::ClearFocus))
    {
        if (AEnemyController* controller = brain->GetEnemyController()) controller->ClearFocusHNS();
    }
    else if (NotifyTag.MatchesTag(Tags::NotifyEvent::EnemyBrain::NotInterruptibleProactive)) bInterruptibleProactive = false;
    else if (NotifyTag.MatchesTag(Tags::NotifyEvent::EnemyBrain::NotInterruptibleReactive)) bInterruptibleReactive = false;
    else if (NotifyTag.MatchesTag(Tags::NotifyEvent::EnemyBrain::SetFocus))
    {
        if (AEnemyController* controller = brain->GetEnemyController()) controller->SetFocusHNS(brain->blackboard.TargetActor);
    }
}

float UEnemySequence::GetTargetDistance() const
{
    if (!brain || !brain->blackboard.TargetActor) return -1.0f;

    AActor* owner = brain->GetOwner();
    if (!owner) return -1.0f;

    FVector startLoc = owner->GetActorLocation();
    FVector endLoc = brain->blackboard.TargetActor->GetActorLocation();
    return FVector::Dist(startLoc, endLoc);
}

bool UEnemySequence::IsFacingActor(AActor* Actor, float Tolerance) const
{
    if (!brain || !Actor) return false;

    AActor* owner = brain->GetOwner();
    if (!owner) return false;

    Tolerance = FMath::Clamp(Tolerance, -1.0f, 1.0f);

    FVector ownerLoc = owner->GetActorLocation();
    FVector targetLoc = Actor->GetActorLocation();

    FVector dirToTarget = (targetLoc - ownerLoc).GetSafeNormal();
    dirToTarget.Z = 0.0f; // Ignore vertical difference for facing check. Design decision: Facing is determined by horizontal only
    
    FVector ownerForward = owner->GetActorForwardVector();
    ownerForward.Z = 0.0f; // Same as above

    return FVector::DotProduct(dirToTarget, ownerForward) >= Tolerance;
}

void UEnemySequence::AddTag(const FGameplayTag& Tag)
{
    if (!brain) return;
    else if (ICombatant* iCombatant = Cast<ICombatant>(brain->GetOwner())) iCombatant->AddTag(Tag);
}

void UEnemySequence::RemoveTag(const FGameplayTag& Tag)
{
    if (!brain) return;
    else if (ICombatant* iCombatant = Cast<ICombatant>(brain->GetOwner())) iCombatant->RemoveTag(Tag);
}

bool UEnemySequence::HasTag(const FGameplayTag& Tag, bool bExact) const
{
    if (!brain) return false;
    else if (ICombatant* iCombatant = Cast<ICombatant>(brain->GetOwner())) return iCombatant->HasTag(Tag, bExact);
    else return false;
}

bool UEnemySequence::HasAnyTags(const TArray<FGameplayTag>& Tags, bool bExact) const
{
    if (!brain) return false;
    else if (ICombatant* iCombatant = Cast<ICombatant>(brain->GetOwner())) return iCombatant->HasAnyTags(Tags, bExact);
    else return false;
}