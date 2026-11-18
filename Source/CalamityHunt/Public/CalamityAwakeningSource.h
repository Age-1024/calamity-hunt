// Copyright (c) 2026 Age-1024. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CalamityAwakeningComponent.h"
#include "CalamityAwakeningSource.generated.h"

class USphereComponent;
class UTextRenderComponent;

/** One-use world event source. Place in a level and configure its mark in the editor. */
UCLASS()
class CALAMITYHUNT_API ACalamityAwakeningSource : public AActor
{
    GENERATED_BODY()

public:
    ACalamityAwakeningSource();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Awakening")
    TObjectPtr<USphereComponent> Trigger;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Awakening")
    TObjectPtr<UTextRenderComponent> Label;

    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Replicated, Category="Awakening")
    ECalamityAwakeningMark MarkToGrant = ECalamityAwakeningMark::Human;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Awakening", meta=(ClampMin="50.0"))
    float UseRadius = 250.0f;

    UPROPERTY(Replicated, BlueprintReadOnly, Category="Awakening")
    bool bConsumed = false;

    /** Must be called by the owning character's server RPC; performs server-side range and state checks. */
    bool TryGrantTo(AActor* ClaimingActor);

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    void OnRep_Consumed();
};
