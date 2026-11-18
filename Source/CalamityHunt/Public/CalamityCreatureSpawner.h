// Copyright (c) 2026 Age-1024. Licensed under the MIT License.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CalamityCreatureSpawner.generated.h"

class ACalamityCreature;
class UTextRenderComponent;
class USceneComponent;

/** Server-side, capped periodic spawner for the graybox creature encounter. */
UCLASS()
class CALAMITYHUNT_API ACalamityCreatureSpawner : public AActor
{
    GENERATED_BODY()
public:
    ACalamityCreatureSpawner();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Spawner")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Spawner")
    TObjectPtr<UTextRenderComponent> Label;

    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Spawner")
    TSubclassOf<ACalamityCreature> CreatureClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spawner", meta=(ClampMin="1.0"))
    float SpawnIntervalSeconds = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spawner", meta=(ClampMin="1"))
    int32 MaxAliveCreatures = 3;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spawner", meta=(ClampMin="0.0"))
    float SpawnRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spawner")
    bool bSpawnDuringScavenging = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spawner")
    bool bSpawnDuringCrownContest = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spawner")
    bool bSpawnDuringBloodMoon = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spawner")
    bool bSpawnImmediately = false;

private:
    FTimerHandle SpawnTimerHandle;
    TArray<TWeakObjectPtr<ACalamityCreature>> SpawnedCreatures;

    void TrySpawnCreature();
    bool IsSpawnPhaseAllowed() const;
    int32 GetAliveSpawnCount();
};
