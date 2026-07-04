#include "CharacterState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../../Tags/AnimNotifyTags.h"
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

/*--------------------------------- UMovementState ---------------------------------*/
FGameplayTag UMovementState::ResolvePlayerInput_Implementation(EPlayerInput PlayerInput, const FVector2D& InputVector)
{
    switch (PlayerInput)
    {
    case EPlayerInput::AttackHeavyStart:
        return CharacterActionTags::AttackHeavyStart;

    case EPlayerInput::AttackHeavyOngoing:
        return CharacterActionTags::AttackHeavyHold;

    case EPlayerInput::AttackHeavyComplete:
        return CharacterActionTags::AttackHeavyRelease;

    case EPlayerInput::AttackLightStart:
        return CharacterActionTags::AttackLightStart;

    case EPlayerInput::AttackLightOngoing:
        return CharacterActionTags::AttackLightHold;

    case EPlayerInput::AttackLightComplete:
        return CharacterActionTags::AttackLightRelease;

    case EPlayerInput::BlockStart:
        return CharacterActionTags::BlockStart;

    case EPlayerInput::BlockTrigger:
        return CharacterActionTags::BlockStart;

    case EPlayerInput::BlockComplete:
        return CharacterActionTags::BlockRelease;

    case EPlayerInput::DodgeStart:
        return CharacterActionTags::Dodge;

    case EPlayerInput::JumpStart:
        return CharacterActionTags::JumpStart;

    case EPlayerInput::JumpComplete:
        return CharacterActionTags::JumpRelease;

    case EPlayerInput::LockOnOffStart:
        return CharacterActionTags::LockOnOffStart;

    case EPlayerInput::LookMouseTrigger:
        return CharacterActionTags::LookMouse;

    case EPlayerInput::LookStickTrigger:
        return CharacterActionTags::LookStick;

    case EPlayerInput::MoveTrigger:
        return CharacterActionTags::Move;

    case EPlayerInput::UseToolsStart:
        return CharacterActionTags::AttackTether;
    
    default:
        return CharacterActionTags::None;
    }
}
/*--------------------------------- UActionState ---------------------------------*/

void UActionState::OnAnimNotify_Implementation(FGameplayTag NotifyTag)
{
    if (NotifyTag.MatchesTagExact(StateMachineTags::ClearActionState)) ownerStateMachineComp->ClearActionState();
}