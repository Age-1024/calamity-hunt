// Copyright (c) 2026 Age-1024. Licensed under the MIT License.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CalamityHealthPickup.generated.h"

class USphereComponent;
class UTextRenderComponent;

/** Server-authoritative health pickup with optional timed respawn. */
UCLASS()
class CALAMITYHUNT_API ACalamityHealthPickup : public AActor
{
    GENERATED_BODY()
public:
    ACalamityHealthPickup();
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Pickup")
    TObjectPtr<USphereComponent> Trigger;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Pickup")
    TObjectPtr<UTextRenderComponent> Label;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pickup", meta=(ClampMin="1.0"))
    float HealAmount = 35.0f;

    /** Zero disables respawn. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pickup", meta=(ClampMin="0.0"))
    float RespawnSeconds = 25.0f;

    UPROPERTY(ReplicatedUsing=OnRep_Available, BlueprintReadOnly, Category="Pickup")
    bool bAvailable = true;

protected:
    UFUNCTION()
    void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void OnRep_Available();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    FTimerHandle RespawnTimerHandle;
    void ApplyAvailability();
    void Respawn();
};
