// Copyright (c) 2026 Age-1024. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CalamityCharacter.generated.h"

UCLASS()
class CALAMITYHUNT_API ACalamityCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ACalamityCharacter();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Replicated, Category = "Combat", meta = (ClampMin = "1.0"))
    float MaxHealth = 100.0f;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Combat")
    float Health = 100.0f;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Combat")
    bool bEliminated = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0.0"))
    float BaseWeaponDamage = 25.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "1.0"))
    float WeaponRange = 10000.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Weapon", meta = (ClampMin = "0.1"))
    float FireIntervalSeconds = 0.25f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Weapon", meta = (ClampMin = "1"))
    int32 MagazineSize = 12;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Combat|Weapon")
    int32 AmmoInMagazine = 12;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Weapon", meta = (ClampMin = "0"))
    int32 InitialReserveAmmo = 48;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Combat|Weapon")
    int32 ReserveAmmo = 48;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Weapon", meta = (ClampMin = "0.1"))
    float ReloadDurationSeconds = 1.5f;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RequestFire();

    UFUNCTION(BlueprintCallable, Category = "Combat|Weapon")
    void RequestReload();

    UFUNCTION(BlueprintCallable, Category = "Interaction|Crown")
    void RequestBeginCrownChannel();

    UFUNCTION(BlueprintCallable, Category = "Interaction|Crown")
    void RequestCancelCrownChannel();

    UFUNCTION(BlueprintCallable, Category = "Interaction|Crown")
    void RequestPickupDroppedCrown();

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool IsEliminated() const { return bEliminated; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    float GetHealth() const { return Health; }

    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    UFUNCTION(Server, Reliable)
    void ServerFire();

    UFUNCTION(Server, Reliable)
    void ServerReload();

    UFUNCTION(Server, Reliable)
    void ServerBeginCrownChannel();

    UFUNCTION(Server, Reliable)
    void ServerCancelCrownChannel();

    UFUNCTION(Server, Reliable)
    void ServerPickupDroppedCrown();

private:
    float LastServerFireTime = -1000.0f;
    bool bIsReloading = false;

    void FireAuthoritative();
    void ReloadAuthoritative();
    void Eliminate();
};
