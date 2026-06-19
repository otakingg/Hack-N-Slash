#include "SpawnAOE.h"
#include "Kismet/GameplayStatics.h"
#include "../../Utility/AOE_Base.h"
#include "../../Interfaces/CombatInstigator.h"

USpawnAOE::USpawnAOE()
{
    #if WITH_EDITORONLY_DATA
        NotifyColor = FColor::Red;
    #endif
}

void USpawnAOE::Notify(USkeletalMeshComponent *MeshComp, UAnimSequenceBase *Animation, const FAnimNotifyEventReference &EventReference)
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
    
    if (ICombatInstigator* iCmbtInst = Cast<ICombatInstigator>(owner)) aoe->SetTarget(iCmbtInst->GetCurrentTarget());

    UGameplayStatics::FinishSpawningActor(aoe, FTransform(FRotator::ZeroRotator, owner->GetActorLocation()));
}
