// CharAnimData.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "FCharAnimData.generated.h"

USTRUCT(BlueprintType)
struct FCharAnimData
{
    GENERATED_BODY()

    // --- Ownership ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Anim|Owner")
    TObjectPtr<class ACharacter> Character = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Anim|Owner")
    TObjectPtr<class UCharacterMovementComponent> MoveComp = nullptr;

    // --- Movement basics ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Anim|Movement")
    FVector VelocityWS = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Anim|Movement")
    FVector AccelWS = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Anim|Movement")
    FVector PrevAccelWS = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Anim|Movement")
    float Speed = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Anim|Movement")
    float Speed2D = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Anim|Movement")
    bool bHasAcceleration = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Anim|Movement")
    bool bStartedMoving = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Anim|Movement")
    bool bIsFalling = false;

    // --- Tags / State context ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Anim|Tags")
    FGameplayTagContainer StateTags;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Anim|Tags", meta = (Tooltip = "Optional extra 'anim-only' context tags"))
    FGameplayTagContainer AnimContextTags;
};