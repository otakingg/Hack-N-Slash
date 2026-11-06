#include "SetMoveMode.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

USetMoveMode::USetMoveMode()
{
    #if WITH_EDITORONLY_DATA
        NotifyColor = FColor(0, 255, 255);
    #endif
}

void USetMoveMode::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    ACharacter* ownerChar = MeshComp->GetOwner<ACharacter>();
    if (!ownerChar) return;

    UCharacterMovementComponent* charMoveComp = ownerChar->GetCharacterMovement();
    if (!charMoveComp) return;

    charMoveComp->SetMovementMode(startMode);
}

void USetMoveMode::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    ACharacter* ownerChar = MeshComp->GetOwner<ACharacter>();
    if (!ownerChar) return;

    UCharacterMovementComponent* charMoveComp = ownerChar->GetCharacterMovement();
    if (!charMoveComp) return;

    charMoveComp->SetMovementMode(endMode);
}