#include "BlockState.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "../../../Structs/FAtkHitData.h"

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

FGameplayTag UBlockState::ResolvePlayerAction_Implementation(const FGameplayTag& PlayerAction, const FVector2D& InputVector)
{
    if (PlayerAction.MatchesTag(Tags::PlayerAction::Move)) return Tags::PlayerAction::Dodge;
    else return PlayerAction;
}