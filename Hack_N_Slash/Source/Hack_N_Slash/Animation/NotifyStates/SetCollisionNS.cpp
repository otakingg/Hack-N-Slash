#include "SetCollisionNS.h"
#include "Components/CapsuleComponent.h"

void USetCollisionNS::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
    if (!IsValid(MeshComp)) return;

    AActor* owner = MeshComp->GetOwner();
    if (!IsValid(owner)) return;

    capsule = owner->FindComponentByClass<UCapsuleComponent>();
    if (!IsValid(capsule)) {return;}
    
    for (ECollisionChannel channel : channlesToBlock)
    {
        responses.Add(channel, capsule->GetCollisionResponseToChannel(channel));
        capsule->SetCollisionResponseToChannel(channel, ECollisionResponse::ECR_Block);
    }

    for (ECollisionChannel channel : channlesToIgnore)
    {
        responses.Add(channel, capsule->GetCollisionResponseToChannel(channel));
        capsule->SetCollisionResponseToChannel(channel, ECollisionResponse::ECR_Ignore);
    }

    for (ECollisionChannel channel : channlesToOverlap)
    {
        responses.Add(channel, capsule->GetCollisionResponseToChannel(channel));
        capsule->SetCollisionResponseToChannel(channel, ECollisionResponse::ECR_Overlap);
    }
}

void USetCollisionNS::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!IsValid(capsule)) {return;}
    for (const TPair<ECollisionChannel, ECollisionResponse>& pair : responses) {capsule->SetCollisionResponseToChannel(pair.Key, pair.Value);}
}