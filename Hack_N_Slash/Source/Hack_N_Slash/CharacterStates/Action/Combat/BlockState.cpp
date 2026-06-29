#include "BlockState.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

#include "../../../Structs/FAtkHitData.h"
#include "../../../Characters/Shared/LocomotionComponent.h"
#include "../../../Tags/LocomotionTags.h"

void UBlockState::EnterState_Implementation()
{
    Super::EnterState_Implementation();
    if (locoComp) locoComp->AddMoveOverrideTag(OverrideTags::Lock);
}

void UBlockState::ExitState_Implementation()
{
    if (locoComp) locoComp->RemoveMoveOverrideTag(OverrideTags::Lock);
    Super::ExitState_Implementation();
}

void UBlockState::ReceiveHit_Implementation(const FAtkHitData& HitData)
{
    Super::ReceiveHit_Implementation(HitData);

    if (!ownerChar) return;

    FaceDamageSource(HitData.damager, HitData.hitLoc);
}

void UBlockState::FaceDamageSource(AActor *Actor, FVector Location)
{
    if (!ownerChar) return;
    else if (Actor)
    {
        FRotator desiredRot = UKismetMathLibrary::FindLookAtRotation(ownerChar->GetActorLocation(), Actor->GetActorLocation());
        desiredRot.Pitch = 0.0f;
        desiredRot.Roll = 0.0f;
        ownerChar->SetActorRotation(desiredRot);
    }
    else
    {
        FRotator desiredRot = UKismetMathLibrary::FindLookAtRotation(ownerChar->GetActorLocation(), Location);
        desiredRot.Pitch = 0.0f;
        desiredRot.Roll = 0.0f;
        ownerChar->SetActorRotation(desiredRot);
    }
}