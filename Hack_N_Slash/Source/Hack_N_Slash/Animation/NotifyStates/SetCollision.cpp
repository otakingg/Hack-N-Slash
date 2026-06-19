#include "SetCollision.h"
#include "Components/CapsuleComponent.h"

USetCollision::USetCollision()
{
    #if WITH_EDITORONLY_DATA
        NotifyColor = FColor(0, 255, 255);
    #endif
}

void USetCollision::NotifyBegin(USkeletalMeshComponent *MeshComp, UAnimSequenceBase *Animation, float FrameDeltaTime, const FAnimNotifyEventReference &EventReference)
{
    if (!MeshComp) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    UCapsuleComponent* capsule = owner->FindComponentByClass<UCapsuleComponent>();
    if (!capsule) return;
    
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

void USetCollision::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    UCapsuleComponent* capsule = owner->FindComponentByClass<UCapsuleComponent>();
    if (!capsule) return;
    
    for (const TPair<ECollisionChannel, ECollisionResponse>& pair : responses) {capsule->SetCollisionResponseToChannel(pair.Key, pair.Value);}
}