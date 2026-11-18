// Copyright (c) 2026 Age-1024. Licensed under the MIT License.

#include "CalamityAwakeningSource.h"
#include "CalamityAwakeningComponent.h"
#include "CalamityCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/TextRenderComponent.h"
#include "Net/UnrealNetwork.h"

ACalamityAwakeningSource::ACalamityAwakeningSource()
{
    bReplicates = true;
    SetReplicateMovement(false);

    Trigger = CreateDefaultSubobject<USphereComponent>(TEXT("Trigger"));
    SetRootComponent(Trigger);
    Trigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Trigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    Trigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    Label = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Label"));
    Label->SetupAttachment(Trigger);
    Label->SetHorizontalAlignment(EHTA_Center);
    Label->SetWorldSize(36.0f);
    Label->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
}

void ACalamityAwakeningSource::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    if (Trigger)
    {
        Trigger->SetSphereRadius(FMath::Max(50.0f, UseRadius));
    }
    if (Label)
    {
        const TCHAR* MarkName = TEXT("觉醒源");
        switch (MarkToGrant)
        {
        case ECalamityAwakeningMark::Human: MarkName = TEXT("人族觉醒源"); break;
        case ECalamityAwakeningMark::Ghost: MarkName = TEXT("鬼族觉醒源"); break;
        case ECalamityAwakeningMark::Zombie: MarkName = TEXT("僵尸觉醒源"); break;
        case ECalamityAwakeningMark::Monster: MarkName = TEXT("怪物觉醒源"); break;
        default: break;
        }
        Label->SetText(FText::FromString(MarkName));
    }
}

void ACalamityAwakeningSource::BeginPlay()
{
    Super::BeginPlay();
    ApplyConsumedState();
}

bool ACalamityAwakeningSource::TryGrantTo(AActor* ClaimingActor)
{
    if (!HasAuthority() || bConsumed || !IsValid(ClaimingActor) ||
        FVector::DistSquared(GetActorLocation(), ClaimingActor->GetActorLocation()) > FMath::Square(FMath::Max(50.0f, UseRadius)))
    {
        return false;
    }

    ACalamityCharacter* Character = Cast<ACalamityCharacter>(ClaimingActor);
    if (!Character || Character->IsEliminated())
    {
        return false;
    }

    UCalamityAwakeningComponent* Awakening = Character->FindComponentByClass<UCalamityAwakeningComponent>();
    if (!Awakening || !Awakening->GrantMark(MarkToGrant))
    {
        return false;
    }

    bConsumed = true;
    ApplyConsumedState(); // RepNotify is not invoked automatically on the authority.
    ForceNetUpdate();
    return true;
}

void ACalamityAwakeningSource::OnRep_Consumed()
{
    ApplyConsumedState();
}

void ACalamityAwakeningSource::ApplyConsumedState()
{
    SetActorHiddenInGame(bConsumed);
    SetActorEnableCollision(!bConsumed);
    if (Trigger)
    {
        Trigger->SetCollisionEnabled(bConsumed ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryOnly);
    }
}

void ACalamityAwakeningSource::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ACalamityAwakeningSource, MarkToGrant);
    DOREPLIFETIME(ACalamityAwakeningSource, bConsumed);
}
