#include "PlayerCamComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"

// Sets default values for this component's properties
UPlayerCamComponent::UPlayerCamComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerCamComponent::BeginPlay()
{
	Super::BeginPlay();
	owner = Cast<ACharacter>(GetOwner());
}

void UPlayerCamComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UPlayerCamComponent::AddLookInputScaled(const FVector2D& Look)
{
	if (!owner) Cast<ACharacter>(GetOwner());
	if (!owner) return;

	UWorld* world = owner->GetWorld();
	if (!world) return;

    const float DT = world->GetDeltaSeconds();

    owner->AddControllerYawInput(Look.X * turnRate * DT);
    owner->AddControllerPitchInput(Look.Y * lookUpRate * DT);
}