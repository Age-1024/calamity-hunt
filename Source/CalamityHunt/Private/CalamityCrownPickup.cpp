// Copyright (c) 2026 Age-1024. Licensed under the MIT License.

#include "CalamityCrownPickup.h"
#include "Components/SphereComponent.h"
#include "Components/TextRenderComponent.h"

ACalamityCrownPickup::ACalamityCrownPickup()
{
    bReplicates = true;
    SetReplicateMovement(true);

    Trigger = CreateDefaultSubobject<USphereComponent>(TEXT("Trigger"));
    SetRootComponent(Trigger);
    Trigger->InitSphereRadius(80.0f);
    Trigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Trigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    Trigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    Label = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Label"));
    Label->SetupAttachment(Trigger);
    Label->SetText(FText::FromString(TEXT("CROWN / 皇印")));
    Label->SetHorizontalAlignment(EHTA_Center);
    Label->SetWorldSize(40.0f);
    Label->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
}
