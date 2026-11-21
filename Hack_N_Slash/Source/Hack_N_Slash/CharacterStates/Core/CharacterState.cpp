#include "CharacterState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../../Animation/AnimInstances/BaseCharAnimInstance.h"
#include "../../Interfaces/Combatant.h"
#include "../../Characters/Shared/LocomotionComponent.h"
#include "../../Characters/Shared/StateMachineComponent.h"

/*--------------------------------- UCharacterState ---------------------------------*/
bool UCharacterState::CanEnterState_Implementation(const UCharacterState* CurrentState) const { return ownerStateMachineComp && iCombatant && ownerChar && moveComp; }

void UCharacterState::Initialize_Implementation(UStateMachineComponent* InSM, ACharacter* InOwner)
{
    if (bInitialized) return;

    ownerStateMachineComp = InSM;
    ownerChar = InOwner;
    if (USkeletalMeshComponent* skeletalMeshComp = ownerChar->GetMesh()) animInst = Cast<UBaseCharAnimInstance>(skeletalMeshComp->GetAnimInstance());
    moveComp = ownerChar ? ownerChar->GetCharacterMovement() : nullptr;
    locoComp = ownerChar ? ownerChar->FindComponentByClass<ULocomotionComponent>() : nullptr;
    iCombatant = Cast<ICombatant>(ownerChar);

    if (ownerStateMachineComp && ownerChar && moveComp) bInitialized = true;
    else UE_LOG(LogTemp, Warning, TEXT("[%s] Initialization failed. StateMachineComp and/or Character and/or CharacterMovement is null"), *GetNameSafe(this));
}

void UCharacterState::EnterState_Implementation()
{
    if (bDebug)
    {
        const FString ClassName = GetNameSafe(this);

        UE_LOG(LogTemp, Log, TEXT("%s: EnterState"), *ClassName);
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, FString::Printf(TEXT("%s: EnterState"), *ClassName));
    }
    AddTag(stateTag);
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
    RemoveTag(stateTag);
}

bool UCharacterState::AddTag(FGameplayTag Tag)
{
    if (!iCombatant) return false;

    iCombatant->AddTag(Tag);
    return true;
}

bool UCharacterState::AddTags(TArray<FGameplayTag> Tags)
{
    for (const FGameplayTag& tag : Tags)
    {
        if (!AddTag(tag)) return false;
    }
    return true;
}

bool UCharacterState::RemoveTag(FGameplayTag Tag)
{
    if (!iCombatant) return false;

    iCombatant->RemoveTag(Tag);
    return true;
}

bool UCharacterState::RemoveTags(TArray<FGameplayTag> Tags)
{
    for (const FGameplayTag& tag : Tags)
    {
        if (!RemoveTag(tag)) return false;
    }
    return true;
}

bool UCharacterState::HasTag(FGameplayTag Tag, bool bExact) const { return iCombatant && iCombatant->HasTag(Tag, bExact); }

bool UCharacterState::HasAnyTags(TArray<FGameplayTag> Tags, bool bExact) const { return iCombatant && iCombatant->HasAnyTags(Tags, bExact); }

/*--------------------------------- UMovementState ---------------------------------*/
FGameplayTag UMovementState::ResolvePlayerInput_Implementation(EPlayerInput PlayerInput, const FVector2D& LookVector, const FVector2D& MoveVector)
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

    case EPlayerInput::AttackNinjutsuStart:
        return Tags::PlayerAction::AttackNinjutsuStart;

    case EPlayerInput::AttackNinjutsuOngoing:
        return Tags::PlayerAction::AttackNinjutsuHold;

    case EPlayerInput::AttackNinjutsuComplete:
        return Tags::PlayerAction::AttackNinjutsuRelease;

    case EPlayerInput::BlockTrigger:
        return Tags::PlayerAction::BlockTrigger;

    case EPlayerInput::BlockStart:
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

    case EPlayerInput::RegenChakraTrigger:
        return Tags::PlayerAction::RegenChakraTrigger;
    
    case EPlayerInput::RegenChakraComplete:
        return Tags::PlayerAction::RegenChakraRelease;
    
    default:
        return Tags::PlayerAction::None;
    }
}
/*--------------------------------- UActionState ---------------------------------*/

void UActionState::OnAnimNotify_Implementation(const FGameplayTag& NotifyTag)
{
    if (NotifyTag.MatchesTagExact(Tags::NotifyEvent::StateMachine::ClearActionState)) ownerStateMachineComp->ClearActionState();
}