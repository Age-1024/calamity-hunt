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

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RequestFire();

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool IsEliminated() const { return bEliminated; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    float GetHealth() const { return Health; }

    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    UFUNCTION(Server, Reliable)
    void ServerFire();

private:
    void FireAuthoritative();
    void Eliminate();
};
