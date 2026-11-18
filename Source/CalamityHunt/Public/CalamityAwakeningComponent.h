// Copyright (c) 2026 Age-1024. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CalamityAwakeningComponent.generated.h"

class ACalamityAwakeningSource;

UENUM(BlueprintType)
enum class ECalamityAwakeningMark : uint8
{
    None UMETA(DisplayName = "None"),
    Human UMETA(DisplayName = "Human"),
    Ghost UMETA(DisplayName = "Ghost"),
    Zombie UMETA(DisplayName = "Zombie"),
    Monster UMETA(DisplayName = "Monster")
};

/** Server-authoritative prototype awakening mark and mobility ability. */
UCLASS(ClassGroup=(Calamity), meta=(BlueprintSpawnableComponent))
class CALAMITYHUNT_API UCalamityAwakeningComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCalamityAwakeningComponent();
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(Replicated, BlueprintReadOnly, Category="Awakening")
    ECalamityAwakeningMark Mark = ECalamityAwakeningMark::None;

    UPROPERTY(Replicated, BlueprintReadOnly, Category="Awakening", meta=(ClampMin="0.0"))
    float Energy = 100.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Awakening", meta=(ClampMin="1.0"))
    float MaxEnergy = 100.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Awakening", meta=(ClampMin="0.0"))
    float EnergyRegenPerSecond = 4.0f;

    UPROPERTY(Replicated, BlueprintReadOnly, Category="Awakening", meta=(ClampMin="0.0"))
    float AbilityCooldownRemaining = 0.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Awakening|Dash", meta=(ClampMin="0.0"))
    float DashEnergyCost = 30.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Awakening|Dash", meta=(ClampMin="0.1"))
    float DashCooldownSeconds = 8.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Awakening|Dash", meta=(ClampMin="0.0"))
    float DashForwardSpeed = 900.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Awakening|Dash", meta=(ClampMin="0.0"))
    float DashVerticalSpeed = 120.0f;

    UFUNCTION(BlueprintCallable, Category="Awakening")
    void RequestClaimSource(ACalamityAwakeningSource* Source);

    UFUNCTION(BlueprintCallable, Category="Awakening")
    void RequestCastAbility();

    /** Called by a source on the server after all source-side checks pass. */
    bool GrantMark(ECalamityAwakeningMark NewMark);

protected:
    UFUNCTION(Server, Reliable)
    void ServerClaimSource(ACalamityAwakeningSource* Source);

    UFUNCTION(Server, Reliable)
    void ServerCastAbility();
};
