#include "CombatantAnimInstance.h"
#include "../../Interfaces/Combatant.h"

void UCombatantAnimInstance::InitializeAnimation()
{
    ownerPawn = TryGetPawnOwner();
    iCombatant = Cast<ICombatant>(ownerPawn);
}

void UCombatantAnimInstance::UpdateAnimation(float DeltaSeconds)
{
    combatantTags.Reset();

    if (!iCombatant) return;

    TMap<FGameplayTag, int32> tags = iCombatant->GetTags();
    for (const TPair<FGameplayTag, int32>& pair : tags) if (pair.Value > 0) combatantTags.AddTag(pair.Key);
}

float UCombatantAnimInstance::PlayMontage(UAnimMontage* Montage, FName Section)
{
    if (!Montage || (Section != NAME_None && !Montage->IsValidSectionName(Section))) return 0.0f;
    
    float const duration = Montage_Play(Montage);
    if (duration > 0.0f && Section != NAME_None) Montage_JumpToSection(Section, Montage);
    return duration;
}