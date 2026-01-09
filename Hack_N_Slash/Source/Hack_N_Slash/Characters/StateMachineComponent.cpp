#include "StateMachineComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UStateMachineComponent::UStateMachineComponent()
{
    // Fully event-driven
    PrimaryComponentTick.bCanEverTick = false;
}

void UStateMachineComponent::BeginPlay()
{
    Super::BeginPlay();

    ownerChar = Cast<ACharacter>(GetOwner());
    InitializeMovementMap();
    InitializeActionMap();

    if (ownerChar)
    {
        ownerChar->LandedDelegate.AddDynamic(this, &UStateMachineComponent::HandleLanded);
        ownerChar->MovementModeChangedDelegate.AddDynamic(this, &UStateMachineComponent::HandleMovementModeChanged);
    }

    // Pick correct baseline at start (ground vs air)
    ApplyBaselineMovement(true);

    if (!currentActionState && defaultActionStateClass)
    {
        if (UActionState* Found = GetActionState(defaultActionStateClass)) ChangeActionState(Found, true);
        else { UE_LOG(LogTemp, Warning, TEXT("[%s] Default Action State not registered: %s"), *GetNameSafe(this), *GetNameSafe(defaultActionStateClass.Get())); }
    }
}

void UStateMachineComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (ownerChar)
    {
        ownerChar->LandedDelegate.RemoveDynamic(this, &UStateMachineComponent::HandleLanded);
        ownerChar->MovementModeChangedDelegate.RemoveDynamic(this, &UStateMachineComponent::HandleMovementModeChanged);
    }
    Super::EndPlay(EndPlayReason);
}
/* ---------------- Initialization ---------------- */

void UStateMachineComponent::InitializeMovementMap()
{
    movementStateInstances.Empty();

    for (const TSubclassOf<UMovementState>& StateClass : movementStateClasses)
    {
        UClass* ClassKey = StateClass.Get();
        if (!ClassKey || ClassKey->HasAnyClassFlags(EClassFlags::CLASS_Abstract)) continue;

        if (movementStateInstances.Contains(ClassKey)) continue; //Prevent duplicates if user accidentally adds same class twice

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

/* ---------------- Transition Rules ---------------- */

bool UStateMachineComponent::CanTransition(const UCharacterState* Current, const UCharacterState* Next, bool bForce)
{
    if (!Next || Next == Current) return false;
    if (bForce) return true;

    if (Current && !Current->CanExitState()) return false;
    if (!Next->CanEnterState(Current)) return false;

    // Priority / interruption rule (mainly for Action, harmless for Movement)
    if (Current && !Current->CanBeInterruptedBy(Next)) return false;

    return true;
}

/* ---------------- State Changes ---------------- */
void UStateMachineComponent::ApplyBaselineMovement(bool bForce)
{
    if (!ownerChar) return;

    UCharacterMovementComponent* MoveComp = ownerChar->GetCharacterMovement();
    if (!MoveComp) return;

    const bool bGrounded = MoveComp->IsMovingOnGround();
    TSubclassOf<UMovementState> DesiredClass = bGrounded ? defaultGroundMovementClass : defaultAirMovementClass;

    if (!DesiredClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] ApplyBaselineMovement: Desired baseline class is not set."), *GetNameSafe(this));
        return;
    }

    UMovementState* Desired = GetMovementState(DesiredClass);
    if (!Desired)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] ApplyBaselineMovement: Baseline not registered: %s"), *GetNameSafe(this), *GetNameSafe(DesiredClass.Get()));
        return;
    }

    // Avoid pointless re-enter unless forced
    if (!bForce && currentMovementState == Desired) return;

    ChangeMovementState(Desired, bForce);
}

void UStateMachineComponent::ChangeState(EStateLayer Layer, UCharacterState* NewState, bool bForce)
{
    if (Layer == EStateLayer::Movement) ChangeMovementState(Cast<UMovementState>(NewState), bForce);
    else ChangeActionState(Cast<UActionState>(NewState), bForce);
}

void UStateMachineComponent::ChangeMovementState(UMovementState* NewState, bool bForce)
{
    if (!CanTransition(currentMovementState, NewState, bForce)) return;

    if (currentMovementState) currentMovementState->ExitState();
    previousMovementState = currentMovementState;
    currentMovementState = NewState;
    currentMovementState->EnterState();
}

void UStateMachineComponent::ChangeActionState(UActionState* NewState, bool bForce)
{
    if (!CanTransition(currentActionState, NewState, bForce)) return;

    if (currentActionState) currentActionState->ExitState();
    previousActionState = currentActionState;
    currentActionState = NewState;
    currentActionState->EnterState();
}

/* ---------------- Tag Queries ---------------- */

UActionState* UStateMachineComponent::GetActionState(TSubclassOf<UActionState> StateClass) const
{
    UClass* ClassKey = StateClass.Get();
    if (!ClassKey) return nullptr;

    if (const TObjectPtr<UActionState>* Found = actionStateInstances.Find(ClassKey)) return Found->Get();
    return nullptr;
}

UMovementState* UStateMachineComponent::GetMovementState(TSubclassOf<UMovementState> StateClass) const
{
    UClass* ClassKey = StateClass.Get();
    if (!ClassKey) return nullptr;

    if (const TObjectPtr<UMovementState>* Found = movementStateInstances.Find(ClassKey)) return Found->Get();
    return nullptr;
}

bool UStateMachineComponent::IsInMovementTag(FGameplayTag Tag) const { return currentMovementState && currentMovementState->GetStateTag().MatchesTag(Tag); }
bool UStateMachineComponent::IsInActionTag(FGameplayTag Tag) const {return currentActionState && currentActionState->GetStateTag().MatchesTag(Tag);}
bool UStateMachineComponent::IsInAnyTag(FGameplayTag Tag) const
{
    // Action layer overrides movement
    if (IsInActionTag(Tag)) return true;
    return IsInMovementTag(Tag);
}

/* ---------------- Event Forwarding ---------------- */
void UStateMachineComponent::HandleLanded(const FHitResult& Hit)
{
    // Baseline likely becomes grounded
    ApplyBaselineMovement(false);

    // Forward event into current movement state
    if (currentMovementState) currentMovementState->OnLanded(Hit); 
}


void UStateMachineComponent::HandleMovementModeChanged(ACharacter* InCharacter, EMovementMode PrevMovementMode, uint8 PrevCustomMode)
{
    ApplyBaselineMovement(false);

    if (currentMovementState) currentMovementState->OnMovementModeChanged(InCharacter, PrevMovementMode, PrevCustomMode);
}

void UStateMachineComponent::OnInputAttackPressed(const FVector2D& InputVector)
{
    if (currentActionState) currentActionState->OnInputAttackPressed(InputVector);
}

void UStateMachineComponent::OnInputBlockDodgePressed(const FVector2D& InputVector)
{
    if (currentActionState) currentActionState->OnInputBlockDodgePressed(InputVector);
}

void UStateMachineComponent::OnInputJumpPressed()
{
    const bool bConsumed = (currentActionState && currentActionState->OnInputJumpPressed());
    if (!bConsumed && currentMovementState) currentMovementState->OnInputJumpPressed();
}

void UStateMachineComponent::OnInputJumpReleased()
{
    const bool bConsumed = (currentActionState && currentActionState->OnInputJumpReleased());
    if (!bConsumed && currentMovementState) currentMovementState->OnInputJumpReleased();
}

void UStateMachineComponent::OnInputLook(const FVector2D& InputVector)
{
    const bool bConsumed = (currentActionState && currentActionState->OnInputLook(InputVector));
    if (!bConsumed && currentMovementState) currentMovementState->OnInputLook(InputVector);
}

void UStateMachineComponent::OnInputMove(const FVector2D& InputVector)
{
    const bool bConsumed = (currentActionState && currentActionState->OnInputMove(InputVector));
    if (!bConsumed && currentMovementState) currentMovementState->OnInputMove(InputVector);
}

void UStateMachineComponent::OnAnimNotify(FName NotifyName)
{
    // Some movement states may care (TurnInPlace), but action usually cares more.
    if (currentActionState)   currentActionState->OnAnimNotify(NotifyName);
    if (currentMovementState) currentMovementState->OnAnimNotify(NotifyName);
}

void UStateMachineComponent::OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
    if (currentActionState)   currentActionState->OnMontageBlendingOut(Montage, bInterrupted);
    if (currentMovementState) currentMovementState->OnMontageBlendingOut(Montage, bInterrupted);
}