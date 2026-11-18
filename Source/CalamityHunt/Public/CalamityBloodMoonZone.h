// Copyright (c) 2026 Age-1024. Licensed under the MIT License.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CalamityBloodMoonZone.generated.h"

class USphereComponent;
class UTextRenderComponent;

/** A level-placed shrinking safe zone. Damage and phase timing are server-authoritative. */
UCLASS()
class CALAMITYHUNT_API ACalamityBloodMoonZone : public AActor
{
    GENERATED_BODY()
public:
    ACalamityBloodMoonZone();
    virtual void Tick(float DeltaSeconds) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Blood Moon")
    TObjectPtr<USphereComponent> RootSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Blood Moon")
    TObjectPtr<UTextRenderComponent> StatusLabel;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Blood Moon|Zone", meta=(ClampMin="100.0"))
    float InitialRadius = 6000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Blood Moon|Zone", meta=(ClampMin="100.0"))
    float FinalRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Blood Moon|Zone", meta=(ClampMin="1.0"))
    float ShrinkDurationSeconds = 240.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Blood Moon|Damage", meta=(ClampMin="0.0"))
    float DamageGraceSeconds = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Blood Moon|Damage", meta=(ClampMin="0.1"))
    float DamageIntervalSeconds = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Blood Moon|Damage", meta=(ClampMin="0.0"))
    float OutsideDamage = 8.0f;

    UPROPERTY(ReplicatedUsing=OnRep_ZoneState, BlueprintReadOnly, Category="Blood Moon")
    float CurrentRadius = 6000.0f;

    UPROPERTY(ReplicatedUsing=OnRep_ZoneState, BlueprintReadOnly, Category="Blood Moon")
    bool bZoneActive = false;

protected:
    UFUNCTION()
    void OnRep_ZoneState();

private:
    float DamageAccumulator = 0.0f;
    float LabelUpdateAccumulator = 0.0f;
    void UpdateLabel();
};
