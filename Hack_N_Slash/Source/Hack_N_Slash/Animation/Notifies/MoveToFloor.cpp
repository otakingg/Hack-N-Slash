#include "MoveToFloor.h"
#include "Kismet/KismetSystemLibrary.h"
#include "../../Characters/Shared/LocomotionComponent.h"

UMoveToFloor::UMoveToFloor()
{
    #if WITH_EDITORONLY_DATA
        NotifyColor = FColor(0, 255, 255);
    #endif
}

void UMoveToFloor::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    ULocomotionComponent* locoComp = owner->FindComponentByClass<ULocomotionComponent>();
    if (!locoComp) return;

    UWorld* world = owner->GetWorld();
    if (!world) return;

	FHitResult outHit;
	FVector startLoc = owner->GetActorLocation();
	FVector endLoc = startLoc + (owner->GetActorUpVector() * traceDistance * -1.0f); // Trace downards
	TArray<AActor*> ignoredActors {owner}; //Ignore self

    if (bDebug) UKismetSystemLibrary::LineTraceSingle(world, startLoc, endLoc, UEngineTypes::ConvertToTraceType(ECC_Visibility), false, ignoredActors, EDrawDebugTrace::ForDuration, outHit, true, FLinearColor::Red, FLinearColor::Green, 0.5f);
    else UKismetSystemLibrary::LineTraceSingle(world, startLoc, endLoc, UEngineTypes::ConvertToTraceType(ECC_Visibility), false, ignoredActors, EDrawDebugTrace::None, outHit, true, FLinearColor::Red, FLinearColor::Green, 0.5f);

    if (outHit.bBlockingHit) locoComp->ApplyRootMotionSourceMoveTo(outHit.TraceEnd, duration, bRestrictSpeedToExpected, velocityOnFinishMode, velocityOnFinish, clampVelocityOnFinish);
    else locoComp->LaunchCharacterHNS({0.0f, 0.0f, -5000.0f}, true, true); // If no floor was found, do this as a fallback
}