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
}

void ACalamityBloodMoonZone::BeginPlay()
{
    Super::BeginPlay();
    if (HasAuthority())
    {
        const float SafeInitialRadius = FMath::Max(100.0f, InitialRadius);
        CurrentRadius = SafeInitialRadius;
        bZoneActive = false;
        BloodMoonElapsedSeconds = 0.0f;
        DamageAccumulator = 0.0f;
    }
    UpdateZonePresentation();
}

void ACalamityBloodMoonZone::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!HasAuthority() || !GetWorld())
    {
        return;
    }

    const ACalamityGameMode* GameMode = GetWorld()->GetAuthGameMode<ACalamityGameMode>();
    ACalamityGameState* GameState = GetWorld()->GetGameState<ACalamityGameState>();
    if (!GameMode || !GameState || GameState->CurrentPhase == ECalamityMatchPhase::Finished)
    {
        if (bZoneActive)
        {
            bZoneActive = false;
            DamageAccumulator = 0.0f;
            UpdateZonePresentation();
            ForceNetUpdate();
        }
        return;
    }

    if (GameState->CurrentPhase != ECalamityMatchPhase::BloodMoon)
    {
        if (bZoneActive)
        {
            bZoneActive = false;
            BloodMoonElapsedSeconds = 0.0f;
            DamageAccumulator = 0.0f;
            UpdateZonePresentation();
            ForceNetUpdate();
        }
        else
        {
            UpdateZonePresentation();
        }
        return;
    }

    BloodMoonElapsedSeconds = FMath::Max(0.0f, GameState->MatchElapsedSeconds - GameMode->BloodMoonStartSeconds);

    const float SafeInitialRadius = FMath::Max(100.0f, InitialRadius);
    const float SafeFinalRadius = FMath::Clamp(FMath::Max(100.0f, FinalRadius), 100.0f, SafeInitialRadius);
    const float ShrinkDuration = FMath::Max(1.0f, ShrinkDurationSeconds);
    const float ShrinkAlpha = FMath::Clamp(BloodMoonElapsedSeconds / ShrinkDuration, 0.0f, 1.0f);
    const float NewRadius = FMath::Lerp(SafeInitialRadius, SafeFinalRadius, ShrinkAlpha);

    const bool bWasActive = bZoneActive;
    const bool bRadiusChanged = !FMath::IsNearlyEqual(CurrentRadius, NewRadius, 1.0f);
    bZoneActive = true;
    CurrentRadius = NewRadius;

    if (bWasActive != bZoneActive || bRadiusChanged)
    {
        UpdateZonePresentation();
        ForceNetUpdate();
    }

    if (BloodMoonElapsedSeconds < FMath::Max(0.0f, DamageGraceSeconds))
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
            UGameplayStatics::ApplyPointDamage(Character, FMath::Max(0.0f, OutsideDamage), Offset.GetSafeNormal(),
                FHitResult(), nullptr, this, NAME_None);
        }
    }
}

void ACalamityBloodMoonZone::OnRep_ZoneState()
{
    UpdateZonePresentation();
}

void ACalamityBloodMoonZone::UpdateZonePresentation()
{
    if (!StatusLabel)
    {
        return;
    }

    if (RootSphere)
    {
        RootSphere->SetSphereRadius(FMath::Max(1.0f, bZoneActive ? CurrentRadius : InitialRadius));
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
    DOREPLIFETIME(ACalamityBloodMoonZone, BloodMoonElapsedSeconds);
}
