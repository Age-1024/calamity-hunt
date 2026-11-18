// Copyright (c) 2026 Age-1024. Licensed under the MIT License.

#include "CalamityAwakeningComponent.h"
#include "CalamityAwakeningSource.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

UCalamityAwakeningComponent::UCalamityAwakeningComponent()
{
    SetIsReplicatedByDefault(true);
    PrimaryComponentTick.bCanEverTick = true;
}

void UCalamityAwakeningComponent::BeginPlay()
{
    Super::BeginPlay();
    if (GetOwner() && GetOwner()->HasAuthority())
    {
        Energy = FMath::Clamp(Energy, 0.0f, MaxEnergy);
    }
}

void UCalamityAwakeningComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (!GetOwner() || !GetOwner()->HasAuthority() || Mark == ECalamityAwakeningMark::None)
    {
        return;
    }
    AbilityCooldownRemaining = FMath::Max(0.0f, AbilityCooldownRemaining - DeltaTime);
    Energy = FMath::Min(MaxEnergy, Energy + FMath::Max(0.0f, EnergyRegenPerSecond) * DeltaTime);
}

void UCalamityAwakeningComponent::RequestClaimSource(ACalamityAwakeningSource* Source)
{
    if (GetOwner() && GetOwner()->HasAuthority())
    {
        ServerClaimSource_Implementation(Source);
    }
    else
    {
        ServerClaimSource(Source);
    }
}

void UCalamityAwakeningComponent::ServerClaimSource_Implementation(ACalamityAwakeningSource* Source)
{
    if (IsValid(Source) && GetOwner() && GetOwner()->HasAuthority())
    {
        Source->TryGrantTo(GetOwner());
    }
}

void UCalamityAwakeningComponent::RequestCastAbility()
{
    if (GetOwner() && GetOwner()->HasAuthority())
    {
        ServerCastAbility_Implementation();
    }
    else
    {
        ServerCastAbility();
    }
}

void UCalamityAwakeningComponent::ServerCastAbility_Implementation()
{
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character || !Character->HasAuthority() || Character->IsActorBeingDestroyed() ||
        Mark == ECalamityAwakeningMark::None || AbilityCooldownRemaining > 0.0f || Energy < DashEnergyCost)
    {
        return;
    }

    const FVector Forward = Character->GetActorForwardVector().GetSafeNormal2D();
    if (Forward.IsNearlyZero())
    {
        return;
    }

    Character->LaunchCharacter(Forward * DashForwardSpeed + FVector::UpVector * DashVerticalSpeed, true, true);
    Energy = FMath::Max(0.0f, Energy - DashEnergyCost);
    AbilityCooldownRemaining = FMath::Max(0.1f, DashCooldownSeconds);
}

bool UCalamityAwakeningComponent::GrantMark(ECalamityAwakeningMark NewMark)
{
    if (!GetOwner() || !GetOwner()->HasAuthority() || NewMark == ECalamityAwakeningMark::None || Mark != ECalamityAwakeningMark::None)
    {
        return false;
    }
    Mark = NewMark;
    Energy = FMath::Clamp(Energy, 0.0f, MaxEnergy);
    return true;
}

void UCalamityAwakeningComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UCalamityAwakeningComponent, Mark);
    DOREPLIFETIME(UCalamityAwakeningComponent, Energy);
    DOREPLIFETIME(UCalamityAwakeningComponent, AbilityCooldownRemaining);
}
