#include "CharacterState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../../Animation/AnimInstances/BaseCharAnimInstance.h"
#include "../../Interfaces/CombatInstigator.h"
#include "../../Characters/Shared/LocomotionComponent.h"
#include "../../Characters/Shared/StateMachineComponent.h"

/*--------------------------------- UCharacterState ---------------------------------*/

void UCharacterState::Initialize_Implementation(UStateMachineComponent* InSM, ACharacter* InOwner)
{
    if (bInitialized) return;

    ownerStateMachineComp = InSM;
    ownerChar = InOwner;
    if (USkeletalMeshComponent* skeletalMeshComp = ownerChar->GetMesh()) animInst = Cast<UBaseCharAnimInstance>(skeletalMeshComp->GetAnimInstance());
    moveComp = ownerChar ? ownerChar->GetCharacterMovement() : nullptr;
    locoComp = ownerChar ? ownerChar->FindComponentByClass<ULocomotionComponent>() : nullptr;
    iCmbtInst = Cast<ICombatInstigator>(ownerChar);

    if (ownerStateMachineComp && ownerChar && moveComp) bInitialized = true;
    else UE_LOG(LogTemp, Warning, TEXT("[%s] Initialization failed. StateMachineComp and/or Character is null"), *GetNameSafe(this));
}

void UCharacterState::EnterState_Implementation()
{
    if (bDebug)
    {
        const FString ClassName = GetNameSafe(this);

        UE_LOG(LogTemp, Log, TEXT("%s: EnterState"), *ClassName);
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, FString::Printf(TEXT("%s: EnterState"), *ClassName));
    }
    iCmbtInst->AddTag(stateTag);
}

void UCharacterState::ExitState_Implementation()
{
    if (bDebug)
    {
        const FString ClassName = GetNameSafe(this);

        UE_LOG(LogTemp, Log, TEXT("%s: ExitState"), *ClassName);
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, FString::Printf(TEXT("%s: ExitState"), *ClassName));
    }
    if (UWorld* world = GetWorld()) world->GetTimerManager().ClearAllTimersForObject(this);
    iCmbtInst->RemoveTag(stateTag);
}

bool UCharacterState::HasGameplayTag(const FGameplayTag& Tag, bool bExact) const { return iCmbtInst && iCmbtInst->HasTag(Tag, bExact); }

bool UCharacterState::HasAnyGameplayTag(const TArray<FGameplayTag>& Tags, bool bExact) const { return iCmbtInst && iCmbtInst->HasAnyTag(Tags, bExact); }

/*--------------------------------- UMovementState ---------------------------------*/
FGameplayTag UMovementState::ResolvePlayerInput_Implementation(EPlayerInput PlayerInput, const FVector2D& InputVector)
{
    switch (PlayerInput)
    {
    case EPlayerInput::AttackHeavyStart:
        return Tags::PlayerAction::AttackHeavyStart;

    case EPlayerInput::AttackHeavyOngoing:
        return Tags::PlayerAction::AttackHeavyHold;

    case EPlayerInput::AttackHeavyComplete:
        return Tags::PlayerAction::AttackHeavyRelease;

    case EPlayerInput::AttackLightStart:
        return Tags::PlayerAction::AttackLightStart;

    case EPlayerInput::AttackLightOngoing:
        return Tags::PlayerAction::AttackLightHold;

    case EPlayerInput::AttackLightComplete:
        return Tags::PlayerAction::AttackLightRelease;

    case EPlayerInput::BlockStart:
        return Tags::PlayerAction::BlockStart;

    case EPlayerInput::BlockTrigger:
        return Tags::PlayerAction::BlockStart;

    case EPlayerInput::BlockComplete:
        return Tags::PlayerAction::BlockRelease;

    case EPlayerInput::DodgeStart:
        return Tags::PlayerAction::Dodge;

    case EPlayerInput::JumpStart:
        return Tags::PlayerAction::JumpStart;

    case EPlayerInput::JumpComplete:
        return Tags::PlayerAction::JumpRelease;

    case EPlayerInput::LockOnOffStart:
        return Tags::PlayerAction::LockOnOffStart;

    case EPlayerInput::LookMouseTrigger:
        return Tags::PlayerAction::LookMouse;

    case EPlayerInput::LookStickTrigger:
        return Tags::PlayerAction::LookStick;

    case EPlayerInput::MoveTrigger:
        return Tags::PlayerAction::Move;

    case EPlayerInput::UseToolsStart:
        return Tags::PlayerAction::AttackTether;
    
    default:
        return Tags::PlayerAction::None;
    }
}
/*--------------------------------- UActionState ---------------------------------*/

void UActionState::OnAnimNotify_Implementation(FGameplayTag NotifyTag)
{
    if (NotifyTag.MatchesTagExact(Tags::NotifyEvent::StateMachine::ClearActionState)) ownerStateMachineComp->ClearActionState();
}