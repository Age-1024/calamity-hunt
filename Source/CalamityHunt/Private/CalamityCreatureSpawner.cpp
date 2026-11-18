// Copyright (c) 2026 Age-1024. Licensed under the MIT License.
#include "CalamityCreatureSpawner.h"
#include "CalamityCreature.h"
#include "CalamityGameState.h"
#include "Components/SceneComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

ACalamityCreatureSpawner::ACalamityCreatureSpawner()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = false;
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);
    Label = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Label"));
    Label->SetupAttachment(SceneRoot);
    Label->SetText(FText::FromString(TEXT("CREATURE SPAWNER / 异灵刷新点")));
    Label->SetHorizontalAlignment(EHTA_Center);
    Label->SetWorldSize(32.0f);
    Label->SetRelativeLocation(FVector(0.0f, 0.0f, 120.0f));
    CreatureClass = ACalamityCreature::StaticClass();
}

void ACalamityCreatureSpawner::BeginPlay()
{
    Super::BeginPlay();
    if (!HasAuthority()) return;
    if (bSpawnImmediately) TrySpawnCreature();
    GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &ACalamityCreatureSpawner::TrySpawnCreature,
        FMath::Max(1.0f, SpawnIntervalSeconds), true);
}

void ACalamityCreatureSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
    Super::EndPlay(EndPlayReason);
}

bool ACalamityCreatureSpawner::IsSpawnPhaseAllowed() const
{
    const ACalamityGameState* State = GetWorld() ? GetWorld()->GetGameState<ACalamityGameState>() : nullptr;
    if (!State) return false;
    switch (State->CurrentPhase)
    {
    case ECalamityMatchPhase::Scavenging: return bSpawnDuringScavenging;
    case ECalamityMatchPhase::CrownContest: return bSpawnDuringCrownContest;
    case ECalamityMatchPhase::BloodMoon: return bSpawnDuringBloodMoon;
    default: return false;
    }
}

int32 ACalamityCreatureSpawner::GetAliveSpawnCount()
{
    SpawnedCreatures.RemoveAll([](const TWeakObjectPtr<ACalamityCreature>& Creature)
    {
        return !Creature.IsValid() || Creature->IsActorBeingDestroyed();
    });
    return SpawnedCreatures.Num();
}

void ACalamityCreatureSpawner::TrySpawnCreature()
{
    if (!HasAuthority() || !GetWorld() || !CreatureClass || !IsSpawnPhaseAllowed() ||
        GetAliveSpawnCount() >= FMath::Max(1, MaxAliveCreatures))
    {
        return;
    }

    FVector Offset = FMath::VRand();
    Offset.Z = 0.0f;
    Offset = Offset.IsNearlyZero() ? FVector::ForwardVector : Offset.GetSafeNormal();
    const float Distance = FMath::FRandRange(0.0f, FMath::Max(0.0f, SpawnRadius));
    const FVector SpawnLocation = GetActorLocation() + Offset * Distance;

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
    ACalamityCreature* Creature = GetWorld()->SpawnActor<ACalamityCreature>(
        CreatureClass, SpawnLocation, GetActorRotation(), Params);
    if (Creature)
    {
        SpawnedCreatures.Add(Creature);
    }
}
