// Copyright (c) 2026 Age-1024. Licensed under the MIT License.
#include "CalamityBloodMoonZone.h"
#include "CalamityCharacter.h"
#include "CalamityGameMode.h"
#include "CalamityGameState.h"
#include "Components/SphereComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

ACalamityBloodMoonZone::ACalamityBloodMoonZone()
{
    PrimaryActorTick.bCanEverTick = true;
    SetActorTickInterval(0.25f);
    bReplicates = true;
    SetReplicateMovement(true);

    RootSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ZoneRoot"));
    SetRootComponent(RootSphere);
    RootSphere->SetSphereRadius(1.0f);
    RootSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    StatusLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("StatusLabel"));
    StatusLabel->SetupAttachment(RootSphere);
    StatusLabel->SetHorizontalAlignment(EHTA_Center);
    StatusLabel->SetWorldSize(48.0f);
    StatusLabel->SetRelativeLocation(FVector(0.0f, 0.0f, 180.0f));
    StatusLabel->SetText(FText::FromString(TEXT("BLOOD MOON / 血月安全区")));
}

void ACalamityBloodMoonZone::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!HasAuthority() || !GetWorld())
    {
        return;
    }

    ACalamityGameMode* GameMode = GetWorld()->GetAuthGameMode<ACalamityGameMode>();
    ACalamityGameState* GameState = GetWorld()->GetGameState<ACalamityGameState>();
    if (!GameMode || !GameState || GameState->CurrentPhase == ECalamityMatchPhase::Finished ||
        GameState->CurrentPhase != ECalamityMatchPhase::BloodMoon)
    {
        if (bZoneActive)
        {
            bZoneActive = false;
            DamageAccumulator = 0.0f;
            OnRep_ZoneState();
            ForceNetUpdate();
        }
        return;
    }

    const float BloodMoonElapsed = FMath::Max(0.0f, GameState->MatchElapsedSeconds - GameMode->BloodMoonStartSeconds);
    const float ShrinkAlpha = FMath::Clamp(BloodMoonElapsed / FMath::Max(1.0f, ShrinkDurationSeconds), 0.0f, 1.0f);
    const float NewRadius = FMath::Lerp(FMath::Max(100.0f, InitialRadius), FMath::Max(100.0f, FinalRadius), ShrinkAlpha);
    const bool bWasActive = bZoneActive;
    const bool bRadiusChanged = !FMath::IsNearlyEqual(CurrentRadius, NewRadius, 1.0f);
    bZoneActive = true;
    CurrentRadius = NewRadius;

    if (bWasActive != bZoneActive || bRadiusChanged)
    {
        OnRep_ZoneState();
        ForceNetUpdate();
    }

    if (BloodMoonElapsed < DamageGraceSeconds)
    {
        DamageAccumulator = 0.0f;
        return;
    }

    DamageAccumulator += DeltaSeconds;
    const float DamageInterval = FMath::Max(0.1f, DamageIntervalSeconds);
    if (DamageAccumulator < DamageInterval)
    {
        return;
    }
    DamageAccumulator = FMath::Fmod(DamageAccumulator, DamageInterval);

    const float RadiusSquared = FMath::Square(CurrentRadius);
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PlayerController = It->Get();
        ACalamityCharacter* Character = PlayerController ? Cast<ACalamityCharacter>(PlayerController->GetPawn()) : nullptr;
        if (!Character || Character->IsEliminated())
        {
            continue;
        }

        const FVector Offset = Character->GetActorLocation() - GetActorLocation();
        const float PlanarDistanceSquared = FMath::Square(Offset.X) + FMath::Square(Offset.Y);
        if (PlanarDistanceSquared > RadiusSquared)
        {
            UGameplayStatics::ApplyDamage(Character, FMath::Max(0.0f, OutsideDamage), nullptr, this, nullptr);
        }
    }

    LabelUpdateAccumulator += DeltaSeconds;
    if (LabelUpdateAccumulator >= 1.0f)
    {
        LabelUpdateAccumulator = 0.0f;
        UpdateLabel();
    }
}

void ACalamityBloodMoonZone::OnRep_ZoneState()
{
    UpdateLabel();
}

void ACalamityBloodMoonZone::UpdateLabel()
{
    if (!StatusLabel)
    {
        return;
    }
    if (!bZoneActive)
    {
        StatusLabel->SetText(FText::FromString(TEXT("BLOOD MOON / 等待血月")));
        return;
    }
    StatusLabel->SetText(FText::FromString(FString::Printf(TEXT("安全区半径 / %.0f"), CurrentRadius)));
}

void ACalamityBloodMoonZone::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ACalamityBloodMoonZone, CurrentRadius);
    DOREPLIFETIME(ACalamityBloodMoonZone, bZoneActive);
}
