#include "SpawnAOE.h"
#include "Kismet/GameplayStatics.h"
#include "../../Utility/AOE_Base.h"
#include "../../Interfaces/CombatInstigator.h"

void USpawnAOE::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp || !aoeClass) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    APawn* pawn = Cast<APawn>(owner);
    if (!pawn) return;

    UWorld* world = owner->GetWorld();
	if (!world) return;

	//Spawn the AOE
    AAOE_Base* aoe = world->SpawnActorDeferred<AAOE_Base>(aoeClass, FTransform(FRotator::ZeroRotator, owner->GetActorLocation()), owner, pawn, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!aoe)
	{
		if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, TEXT("Spawn AOE: Failed"));
		return;
	}

    aoe->SetDebug(bDebug);
    aoe->SetIgnoreSelf(bIgnoreSelf);
    aoe->SetRadius(radius);
    
    if (ICombatInstigator* iCmbtInst = Cast<ICombatInstigator>(owner))
    {
        if (AActor* target = iCmbtInst->GetCurrentTarget()) aoe->SetTarget(target);
    }

    UGameplayStatics::FinishSpawningActor(aoe, FTransform(FRotator::ZeroRotator, owner->GetActorLocation()));
}
