// Copyright (c) 2026 Age-1024. Licensed under the MIT License.
#include "CalamityHealthPickup.h"
#include "CalamityCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/TextRenderComponent.h"
#include "Net/UnrealNetwork.h"

ACalamityHealthPickup::ACalamityHealthPickup()
{
    bReplicates = true;
    SetReplicateMovement(false);

    Trigger = CreateDefaultSubobject<USphereComponent>(TEXT("Trigger"));
    SetRootComponent(Trigger);
    Trigger->InitSphereRadius(90.0f);
    Trigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Trigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    Trigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    Trigger->OnComponentBeginOverlap.AddDynamic(this, &ACalamityHealthPickup::OnOverlap);

    Label = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Label"));
    Label->SetupAttachment(Trigger);
    Label->SetText(FText::FromString(TEXT("MEDKIT / 急救包")));
    Label->SetHorizontalAlignment(EHTA_Center);
    Label->SetWorldSize(32.0f);
    Label->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
}

void ACalamityHealthPickup::BeginPlay()
{
    Super::BeginPlay();
    ApplyAvailability();
}

void ACalamityHealthPickup::OnOverlap(UPrimitiveComponent*, AActor* OtherActor,
    UPrimitiveComponent*, int32, bool, const FHitResult&)
{
    if (!HasAuthority() || !bAvailable)
    {
        return;
    }

    ACalamityCharacter* Character = Cast<ACalamityCharacter>(OtherActor);
    if (!Character || Character->IsEliminated() || Character->Health >= Character->MaxHealth)
    {
        return;
    }

    const float PreviousHealth = Character->Health;
    Character->Health = FMath::Min(Character->MaxHealth, Character->Health + FMath::Max(0.0f, HealAmount));
    if (Character->Health <= PreviousHealth)
    {
        return;
    }

    bAvailable = false;
    ApplyAvailability(); // RepNotify only runs automatically on clients.
    ForceNetUpdate();

    if (RespawnSeconds > 0.0f)
    {
        GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &ACalamityHealthPickup::Respawn,
            RespawnSeconds, false);
    }
}

void ACalamityHealthPickup::OnRep_Available()
{
    ApplyAvailability();
}

void ACalamityHealthPickup::ApplyAvailability()
{
    SetActorHiddenInGame(!bAvailable);
    SetActorEnableCollision(bAvailable);
    if (Trigger)
    {
        Trigger->SetCollisionEnabled(bAvailable ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
    }
}

void ACalamityHealthPickup::Respawn()
{
    if (!HasAuthority())
    {
        return;
    }
    bAvailable = true;
    ApplyAvailability();
    ForceNetUpdate();
}

void ACalamityHealthPickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ACalamityHealthPickup, bAvailable);
}
