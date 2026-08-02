#include "EnemySequence.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../../../Animation/AnimInstances/BaseCharAnimInstance.h"
#include "../../../Interfaces/CombatInstigator.h"
#include "../EnemyBrainComponent.h"
#include "../../../Controllers/EnemyController.h"
#include "../../../Utility/Tags.h"

void UEnemySequence::Initialize_Implementation(UEnemyBrainComponent* InBrain)
{
    brain = InBrain;

    if (bStartOnCooldown && cooldown > 0.0f)
    {
        if (UWorld* world = GetWorld())
        {
            bOnCooldown = true;
            world->GetTimerManager().SetTimer(TH_Cooldown, this, &UEnemySequence::EndCooldown, cooldown, false);
        }
    }
}

bool UEnemySequence::IsActive() const { return brain && brain->GetActiveSequence() == this; }

void UEnemySequence::Finish_Implementation() { FinishHelper(); }

void UEnemySequence::FinishHelper()
{
    if (!brain) return;

    UWorld* world = GetWorld();
    if (!world) return;

    lastSequenceTime = world->GetTimeSeconds();
    brain->RemoveActiveSequence();
    sequenceIndex = 1;
    bInterruptible = false;

    if (cooldown > 0.0f)
    {
        bOnCooldown = true;
        FTimerManager& timerManager = world->GetTimerManager();
        timerManager.ClearAllTimersForObject(this);
        timerManager.SetTimer(TH_Cooldown, this, &UEnemySequence::EndCooldown, cooldown, false);
    }
}

void UEnemySequence::Abort_Implementation() { AbortHelper(); }

void UEnemySequence::AbortHelper()
{
    if (!brain) return;

    UWorld* world = GetWorld();
    if (!world) return;

    lastSequenceTime = world->GetTimeSeconds();
    sequenceIndex = 1;
    bInterruptible = false;

    if (AEnemyController* controller = brain->GetEnemyController())
    {
        controller->StopMovement();
        controller->ClearFocusHNS();
    }

    SetMovementMode(EMovementMode::MOVE_Walking);

    if (UBaseCharAnimInstance* animInst = brain->GetAnimInstance()) animInst->Montage_Stop(0.25f);
    
    world->GetTimerManager().ClearAllTimersForObject(this);
}

void UEnemySequence::HandleAnimNotify_Implementation(const FGameplayTag& NotifyTag)
{
    if (!IsActive()) return;
    else if (NotifyTag.MatchesTag(Tags::NotifyEvent::EnemyBrain::AdvanceSequence)) AdvanceSequence();
    else if (NotifyTag.MatchesTag(Tags::NotifyEvent::EnemyBrain::ClearFocus))
    {
        if (!brain) return;
        if (AEnemyController* controller = brain->GetEnemyController()) controller->ClearFocusHNS();
    }
    else if (NotifyTag.MatchesTag(Tags::NotifyEvent::EnemyBrain::NotInterruptible)) bInterruptible = false;
    else if (NotifyTag.MatchesTag(Tags::NotifyEvent::EnemyBrain::SetFocus))
    {
        if (!brain) return;
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

void UEnemySequence::AddTag(const FGameplayTag& Tag)
{
    if (!IsActive()) return;
    if (ICombatInstigator* iCmbtInst = Cast<ICombatInstigator>(brain->GetOwner())) iCmbtInst->AddTag(Tag);
}

void UEnemySequence::RemoveTag(const FGameplayTag& Tag)
{
    if (!IsActive()) return;
    if (ICombatInstigator* iCmbtInst = Cast<ICombatInstigator>(brain->GetOwner())) iCmbtInst->RemoveTag(Tag);
}

bool UEnemySequence::HasTag(const FGameplayTag& Tag, bool bExact) const
{
    if (ICombatInstigator* iCmbtInst = Cast<ICombatInstigator>(brain->GetOwner())) return iCmbtInst->HasTag(Tag, bExact);
    else return false;
}

bool UEnemySequence::HasAnyTag(const TArray<FGameplayTag>& Tags, bool bExact) const
{
    if (ICombatInstigator* iCmbtInst = Cast<ICombatInstigator>(brain->GetOwner())) return iCmbtInst->HasAnyTag(Tags, bExact);
    else return false;
}

void UEnemySequence::SetWalkSpeedAndAcceleration(float WalkSpeed, float Acceleration)
{
    if (!IsActive()) return;

    UCharacterMovementComponent* moveComp = brain->GetCharacterMovement();
    if (!moveComp) return;

    moveComp->MaxWalkSpeed = WalkSpeed;
    moveComp->MaxAcceleration = Acceleration;
}

void UEnemySequence::SetFlySpeedAndAcceleration(float FlySpeed, float Acceleration)
{
    if (!IsActive()) return;

    UCharacterMovementComponent* moveComp = brain->GetCharacterMovement();
    if (!moveComp) return;

    moveComp->MaxFlySpeed = FlySpeed;
    moveComp->MaxAcceleration = Acceleration;
}

void UEnemySequence::SetMovementMode(EMovementMode NewMode, uint8 CustomMode)
{
    if (!IsActive()) return;
    if (UCharacterMovementComponent* moveComp = brain->GetCharacterMovement()) moveComp->SetMovementMode(NewMode, CustomMode);
}