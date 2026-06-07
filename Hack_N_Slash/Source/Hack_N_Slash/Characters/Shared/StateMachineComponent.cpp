#include "StateMachineComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../../Tags/CharacterStateTagNamespaces.h"
#include "../../Interfaces/Damageable.h"
#include "../../Structs/FAtkHitData.h"
#include "../../Interfaces/LocomotionCmdInterface.h"
#include "../../Interfaces/CombatCmdInterface.h"

UStateMachineComponent::UStateMachineComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UStateMachineComponent::BeginPlay()
{
    Super::BeginPlay();
    
    ownerChar = Cast<ACharacter>(GetOwner());

    CacheCommandInterfaces();
    InitializeMovementMap();
    InitializeActionMap();

    if (ownerChar)
    {
        ownerChar->LandedDelegate.AddDynamic(this, &UStateMachineComponent::HandleLanded);
        ownerChar->MovementModeChangedDelegate.AddDynamic(this, &UStateMachineComponent::HandleMovementModeChanged);
        ownerChar->OnReachedJumpApex.AddDynamic(this, &UStateMachineComponent::HandleJumpApexReached);
    }

    DecideMovementState(true);

    UActionState* desiredState = GetActionStateByTag(defaultActionTag);
    if (!desiredState && bDebug) UE_LOG(LogTemp, Warning, TEXT("[%s] Default Action State not registered: %s"), *GetNameSafe(this), *defaultActionTag.ToString());
    ChangeActionState(desiredState, true);

    RebuildActiveStateTags(); // Ensure tags are correct immediately
}

void UStateMachineComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (ownerChar)
    {
        ownerChar->LandedDelegate.RemoveDynamic(this, &UStateMachineComponent::HandleLanded);
        ownerChar->MovementModeChangedDelegate.RemoveDynamic(this, &UStateMachineComponent::HandleMovementModeChanged);
        ownerChar->OnReachedJumpApex.RemoveDynamic(this, &UStateMachineComponent::HandleJumpApexReached);
    }

    Super::EndPlay(EndPlayReason);
}

// ---------------------- Cache Interfaces ------------------
void UStateMachineComponent::CacheCommandInterfaces()
{
    if (!ownerChar) return;

    iLocomotionCmd = nullptr;
    iCombatCmd     = nullptr;

    // ---------------- Locomotion ----------------

    TArray<UActorComponent*> LocoComps = ownerChar->GetComponentsByInterface(ULocomotionCmdInterface::StaticClass());
    if (LocoComps.Num() > 0) iLocomotionCmd = Cast<ILocomotionCmdInterface>(LocoComps[0]);
    else if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("StateMachineComp: CacheCmdInterfaces: No locomotion cmd interface found"));

    // ---------------- Combat ----------------

    TArray<UActorComponent*> CombatComps = ownerChar->GetComponentsByInterface(UCombatCmdInterface::StaticClass());
    if (CombatComps.Num() > 0) iCombatCmd = Cast<ICombatCmdInterface>(CombatComps[0]);
    else if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("StateMachineComp: CacheCmdInterfaces: No combat cmd interface found"));
}

// ---------------- Initialization ----------------
void UStateMachineComponent::InitializeMovementMap()
{
    movementStateInstances.Empty();

    for (const TSubclassOf<UMovementState>& StateClass : movementStateClasses)
    {
        UClass* ClassKey = StateClass.Get();
        if (!ClassKey || ClassKey->HasAnyClassFlags(EClassFlags::CLASS_Abstract)) continue;

        if (movementStateInstances.Contains(ClassKey)) continue; // Prevent duplicates

        UMovementState* Instance = NewObject<UMovementState>(this, ClassKey);
        if (!Instance) continue;

        Instance->Initialize(this, ownerChar);
        movementStateInstances.Add(ClassKey, Instance);
    }
}

void UStateMachineComponent::InitializeActionMap()
{
    actionStateInstances.Empty();

    for (const TSubclassOf<UActionState>& StateClass : actionStateClasses)
    {
        UClass* ClassKey = StateClass.Get();
        if (!ClassKey || ClassKey->HasAnyClassFlags(EClassFlags::CLASS_Abstract)) continue;

        if (actionStateInstances.Contains(ClassKey)) continue;

        UActionState* Instance = NewObject<UActionState>(this, ClassKey);
        if (!Instance) continue;

        Instance->Initialize(this, ownerChar);
        actionStateInstances.Add(ClassKey, Instance);
    }
}

ICombatCmdInterface* UStateMachineComponent::GetCombatCommands() const { return iCombatCmd; }
ILocomotionCmdInterface* UStateMachineComponent::GetLocomotionCommands() const { return iLocomotionCmd; }

// ---------------- State Lookup ----------------
UActionState* UStateMachineComponent::GetActionStateByTag(const FGameplayTag& Tag) const
{
    if (!Tag.IsValid()) return nullptr;

    for (const auto& Pair : actionStateInstances)
    {
        UActionState* State = Pair.Value;
        if (State && State->HasExactStateTag(Tag)) return State;
    }

    return nullptr;
}

UMovementState* UStateMachineComponent::GetMovementStateByTag(const FGameplayTag& Tag) const
{
    if (!Tag.IsValid()) return nullptr;
    
    for (const auto& Pair : movementStateInstances)
    {
        UMovementState* State = Pair.Value;
        if (State && State->HasExactStateTag(Tag)) return State;
    }

    return nullptr;
}

// ---------------- Tag Queries ----------------
void UStateMachineComponent::RebuildActiveStateTags()
{
    activeStateTags.Reset();

    if (currentMovementState) activeStateTags.AddTag(currentMovementState->GetStateTag());
    if (currentActionState) activeStateTags.AddTag(currentActionState->GetStateTag());
}

bool UStateMachineComponent::HasActiveTag(const FGameplayTag& Tag) const { return activeStateTags.HasTag(Tag); }
bool UStateMachineComponent::HasExactActiveTag(const FGameplayTag& Tag) const { return activeStateTags.HasTagExact(Tag); }
bool UStateMachineComponent::IsInMovementTag(const FGameplayTag& Tag) const { return currentMovementState && currentMovementState->HasStateTag(Tag); }
bool UStateMachineComponent::IsInExactMovementTag(const FGameplayTag &Tag) const { return currentMovementState && currentMovementState->HasExactStateTag(Tag); }
bool UStateMachineComponent::IsInActionTag(const FGameplayTag &Tag) const { return currentActionState && currentActionState->HasStateTag(Tag); }
bool UStateMachineComponent::IsInExactActionTag(const FGameplayTag &Tag) const { return currentActionState && currentActionState->HasExactStateTag(Tag); }

bool UStateMachineComponent::IsAirborne() const { return HasActiveTag(airborneTag); }
bool UStateMachineComponent::IsGrounded() const { return HasActiveTag(groundedTag); }
/* ---------------- Transition Rules ---------------- */

bool UStateMachineComponent::CanTransition(const UCharacterState* Current, const UCharacterState* Next, bool bForce)
{
    if (!Next || Next == Current) return false;
    if (bForce) return true;

    if (Current && !Current->CanExitState()) return false;
    if (!Next->CanEnterState(Current)) return false;

    return true;
}

/* ---------------- State changes ---------------- */
bool UStateMachineComponent::ChangeMovementState(UMovementState* NewState, bool bForce)
{
    if (!CanTransition(currentMovementState, NewState, bForce)) return false;

    if (currentMovementState) currentMovementState->ExitState();
    previousMovementState = currentMovementState;
    currentMovementState = NewState;
    currentMovementState->EnterState();

    RebuildActiveStateTags();
    return true;
}

bool UStateMachineComponent::ChangeActionState(UActionState* NewState, bool bForce)
{
    if (!CanTransition(currentActionState, NewState, bForce)) return false;

    if (currentActionState) currentActionState->ExitState();
    previousActionState = currentActionState;
    currentActionState = NewState;
    currentActionState->EnterState();

    RebuildActiveStateTags();
    return true;
}

void UStateMachineComponent::ClearActionState()
{
    UActionState* noneState = GetActionStateByTag(ActionTags::None);
    ChangeActionState(noneState, true);
}

void UStateMachineComponent::DecideMovementState(bool bForce)
{
    for (auto& pair : movementStateInstances) if (ChangeMovementState(pair.Value, bForce)) break;
}

/* ---------------- Unified Requests ---------------- */

void UStateMachineComponent::RequestAttackPlayer(const FVector2D& InputVector, EPlayerAction PlayerAction)
{
    if (currentMovementState && currentMovementState->OnAttackIntent(InputVector, PlayerAction)) return;
    if (currentActionState) currentActionState->OnAttackIntent(InputVector, PlayerAction);
}

void UStateMachineComponent::RequestAttackEnemy(const FEnemyAtkData& AtkData)
{
    if (currentMovementState && currentMovementState->OnAttackIntent(AtkData)) return;
    if (currentActionState) currentActionState->OnAttackIntent(AtkData);
}

void UStateMachineComponent::RequestBlockStart()
{
    if (currentMovementState && currentMovementState->OnBlockStartIntent()) return;
    if (currentActionState) currentActionState->OnBlockStartIntent();
}

void UStateMachineComponent::RequestBlockStop()
{
    if (currentMovementState && currentMovementState->OnBlockStopIntent()) return;
    if (currentActionState) currentActionState->OnBlockStopIntent();
}

void UStateMachineComponent::RequestDodge(const FVector2D& InputVector)
{
    if (currentMovementState && currentMovementState->OnDodgeIntent(InputVector)) return;
    if (currentActionState) currentActionState->OnDodgeIntent(InputVector);
}

void UStateMachineComponent::RequestJumpStart()
{
    if (currentMovementState && currentMovementState->OnJumpStartIntent()) return;
    if (currentActionState) currentActionState->OnJumpStartIntent();
}

void UStateMachineComponent::RequestJumpStop()
{
    if (currentMovementState && currentMovementState->OnJumpStopIntent()) return;
    if (currentActionState) currentActionState->OnJumpStopIntent();
}

void UStateMachineComponent::RequestLookMouse(const FVector2D& InputVector)
{
    if (currentMovementState && currentMovementState->OnLookMouseIntent(InputVector)) return;
    if (currentActionState) currentActionState->OnLookMouseIntent(InputVector);
}

void UStateMachineComponent::RequestLookStick(const FVector2D& InputVector)
{
    if (currentMovementState && currentMovementState->OnLookStickIntent(InputVector)) return;
    if (currentActionState) currentActionState->OnLookStickIntent(InputVector);
}

void UStateMachineComponent::RequestMove(const FVector2D& InputVector)
{
    if (currentActionState && currentActionState->OnMoveIntent(InputVector)) return;
    if (currentMovementState) currentMovementState->OnMoveIntent(InputVector);
}

void UStateMachineComponent::RequestMoveTo(AActor* Target, const FVector Loc, float AcceptanceRadius)
{
    if (currentActionState && currentActionState->OnMoveIntent(Target, Loc, AcceptanceRadius)) return;
    if (currentMovementState) currentMovementState->OnMoveIntent(Target, Loc, AcceptanceRadius);
}

void UStateMachineComponent::RequestToggleLockOn()
{
    if (currentMovementState && currentMovementState->OnToggleLockOnIntent()) return;
    if (currentActionState) currentActionState->OnToggleLockOnIntent();
}

/* ---------------- Character / Anim forwarding (unchanged) ---------------- */

void UStateMachineComponent::HandleJumpApexReached()
{
    if (currentActionState) currentActionState->OnJumpApexReached();
    if (currentMovementState) currentMovementState->OnJumpApexReached();
}

void UStateMachineComponent::HandleLanded(const FHitResult& Hit)
{
    if (currentActionState) currentActionState->OnLanded(Hit);

    // 1) Old state (air) reacts first
    if (currentMovementState) currentMovementState->OnLanded(Hit);

    // 2) Swap baseline
    DecideMovementState(false);

    // 3) Let the new baseline react too
    if (currentMovementState) currentMovementState->OnLanded(Hit);
}

void UStateMachineComponent::HandleMovementModeChanged(ACharacter* InCharacter, EMovementMode PrevMovementMode, uint8 PrevCustomMode)
{
    if (currentActionState) currentActionState->OnMovementModeChanged(InCharacter, PrevMovementMode, PrevCustomMode);
    if (currentMovementState) currentMovementState->OnMovementModeChanged(InCharacter, PrevMovementMode, PrevCustomMode);
    DecideMovementState(false);
}

void UStateMachineComponent::HandleAnimNotify(FGameplayTag NotifyTag)
{
    if (currentActionState)   currentActionState->OnAnimNotify(NotifyTag);
    if (currentMovementState) currentMovementState->OnAnimNotify(NotifyTag);
}

/* -------------------- Combat Forwarding -----------------------*/

void UStateMachineComponent::HandleReceiveHit(const FAtkHitData& HitData)
{
    UActionState* reactionState = nullptr;

    if (IDamageable* iDmgble = Cast<IDamageable>(ownerChar))
    {
        if (!iDmgble->IsAlive()) reactionState = GetActionStateByTag(ReactionTags::Dead);
        else reactionState = GetActionStateByTag(ReactionTags::Hit);
    }
    else reactionState = GetActionStateByTag(ReactionTags::Hit);

    if (!reactionState) return;

    ChangeActionState(reactionState, false);
    if (currentActionState) currentActionState->ReceiveHit(HitData);
}

void UStateMachineComponent::HandleCountered(AActor* Counteror, const FString& Reason)
{
    UActionState* hitState = GetActionStateByTag(ReactionTags::Hit);
    if (!hitState) return;

    ChangeActionState(hitState, false);

    if (currentActionState)
    {
        FAtkHitData hitData = FAtkHitData();
        hitData.attacker = Counteror;
        hitData.damager = Counteror;
        hitData.resolvedReaction = ReactionTags::Countered;

        currentActionState->ReceiveHit(hitData);
    }
}