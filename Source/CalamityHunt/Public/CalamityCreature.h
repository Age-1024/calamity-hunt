// Copyright (c) 2026 Age-1024. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CalamityCreature.generated.h"

class APlayerState;
class AController;

UCLASS()
class CALAMITYHUNT_API ACalamityCreature : public ACharacter
{
    GENERATED_BODY()

public:
    ACalamityCreature();
    virtual void Tick(float DeltaSeconds) override;
    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Creature", meta = (ClampMin = "1.0"))
    float MaxHealth = 120.0f;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Creature")
    float Health = 120.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Creature", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TameHealthFraction = 0.5f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Creature", meta = (ClampMin = "1.0"))
    float MoveSpeed = 320.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Creature|Combat", meta = (ClampMin = "1.0"))
    float AttackRange = 160.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Creature|Combat", meta = (ClampMin = "0.1"))
    float AttackInterval = 1.5f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Creature|Combat", meta = (ClampMin = "0.0"))
    float AttackDamage = 12.0f;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Creature")
    TObjectPtr<APlayerState> TamedBy = nullptr;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Creature")
    bool bIsDead = false;

    UFUNCTION(BlueprintPure, Category = "Creature")
    bool IsTamed() const { return TamedBy != nullptr; }

    UFUNCTION(BlueprintCallable, Category = "Creature")
    bool TryTame(AController* Tamer);

private:
    float AttackCooldown = 0.0f;
    AActor* FindTarget() const;
    void HandleDeath();
};
