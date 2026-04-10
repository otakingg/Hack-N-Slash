#include "PlayerTargettingComponent.h"

UPlayerTargettingComponent::UPlayerTargettingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerTargettingComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPlayerTargettingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}