// Copyright (c) 2026 Age-1024. Licensed under the MIT License.

#include "CalamityAwakeningSource.h"
#include "CalamityAwakeningComponent.h"
#include "Components/SphereComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

ACalamityAwakeningSource::ACalamityAwakeningSource()
{
    bReplicates = true;
    SetReplicateMovement(false);

    Trigger = CreateDefaultSubobject<USphereComponent>(TEXT("Trigger"));
    SetRootComponent(Trigger);
    Trigger->InitSphereRadius(UseRadius);
    Trigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Trigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    Trigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    Label = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Label"));
    Label->SetupAttachment(Trigger);
    Label->SetText(FText::FromString(TEXT("AWAKENING / 觉醒源")));
    Label->SetHorizontalAlignment(EHTA_Center);
    Label->SetWorldSize(36.0f);
    Label->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
}

bool ACalamityAwakeningSource::TryGrantTo(AActor* ClaimingActor)
{
    if (!HasAuthority() || bConsumed || !IsValid(ClaimingActor) ||
        FVector::DistSquared(GetActorLocation(), ClaimingActor->GetActorLocation()) > FMath::Square(UseRadius))
    {
        return false;
    }

    ACharacter* Character = Cast<ACharacter>(ClaimingActor);
    UCalamityAwakeningComponent* Awakening = Character ? Character->FindComponentByClass<UCalamityAwakeningComponent>() : nullptr;
    if (!Awakening || !Awakening->GrantMark(MarkToGrant))
    {
        return false;
    }

    bConsumed = true;
    OnRep_Consumed();
    return true;
}

void ACalamityAwakeningSource::OnRep_Consumed()
{
    SetActorHiddenInGame(bConsumed);
    SetActorEnableCollision(!bConsumed);
}

void ACalamityAwakeningSource::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ACalamityAwakeningSource, MarkToGrant);
    DOREPLIFETIME(ACalamityAwakeningSource, bConsumed);
}
