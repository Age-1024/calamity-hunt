// Copyright (c) 2026 Age-1024. Licensed under the MIT License.

#include "CalamityCharacter.h"
#include "CalamityGameMode.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

ACalamityCharacter::ACalamityCharacter()
{
    bReplicates = true;
    SetReplicateMovement(true);
    Health = MaxHealth;
}

void ACalamityCharacter::RequestFire()
{
    if (bEliminated)
    {
        return;
    }
    if (HasAuthority())
    {
        FireAuthoritative();
    }
    else
    {
        ServerFire();
    }
}

void ACalamityCharacter::ServerFire_Implementation()
{
    FireAuthoritative();
}

void ACalamityCharacter::FireAuthoritative()
{
    if (!HasAuthority() || bEliminated || !GetWorld())
    {
        return;
    }

    const FVector Start = GetPawnViewLocation();
    const FVector Direction = GetBaseAimRotation().Vector();
    const FVector End = Start + Direction * WeaponRange;

    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(CalamityHitscan), true, this);
    QueryParams.AddIgnoredActor(this);
    FHitResult Hit;
    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, QueryParams) && Hit.GetActor())
    {
        UGameplayStatics::ApplyPointDamage(
            Hit.GetActor(), BaseWeaponDamage, Direction, Hit, GetController(), this, nullptr);
    }
}

float ACalamityCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if (!HasAuthority() || bEliminated || DamageAmount <= 0.0f)
    {
        return 0.0f;
    }

    const float AppliedDamage = FMath::Min(Health, DamageAmount);
    Health = FMath::Max(0.0f, Health - AppliedDamage);
    if (Health <= 0.0f)
    {
        Eliminate();
    }
    return AppliedDamage;
}

void ACalamityCharacter::Eliminate()
{
    if (!HasAuthority() || bEliminated)
    {
        return;
    }

    bEliminated = true;
    SetActorEnableCollision(false);
    if (ACalamityGameMode* GameMode = GetWorld()->GetAuthGameMode<ACalamityGameMode>())
    {
        GameMode->HandlePlayerEliminated(GetController());
    }
}

void ACalamityCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ACalamityCharacter, MaxHealth);
    DOREPLIFETIME(ACalamityCharacter, Health);
    DOREPLIFETIME(ACalamityCharacter, bEliminated);
}
