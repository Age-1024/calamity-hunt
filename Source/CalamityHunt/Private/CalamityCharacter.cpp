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
    AmmoInMagazine = MagazineSize;
    ReserveAmmo = InitialReserveAmmo;
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

void ACalamityCharacter::RequestReload()
{
    if (HasAuthority())
    {
        ReloadAuthoritative();
    }
    else
    {
        ServerReload();
    }
}

void ACalamityCharacter::ServerReload_Implementation()
{
    ReloadAuthoritative();
}

void ACalamityCharacter::ReloadAuthoritative()
{
    if (!HasAuthority() || bEliminated || bIsReloading || AmmoInMagazine >= MagazineSize || ReserveAmmo <= 0)
    {
        return;
    }

    bIsReloading = true;
    FTimerHandle ReloadTimer;
    TWeakObjectPtr<ACalamityCharacter> WeakThis(this);
    GetWorldTimerManager().SetTimer(ReloadTimer, [WeakThis]()
    {
        ACalamityCharacter* Character = WeakThis.Get();
        if (!Character || !Character->HasAuthority())
        {
            return;
        }
        Character->bIsReloading = false;
        if (Character->bEliminated)
        {
            return;
        }
        const int32 Needed = Character->MagazineSize - Character->AmmoInMagazine;
        const int32 Loaded = FMath::Min(Needed, Character->ReserveAmmo);
        Character->AmmoInMagazine += Loaded;
        Character->ReserveAmmo -= Loaded;
    }, ReloadDurationSeconds, false);
}

void ACalamityCharacter::RequestBeginCrownChannel()
{
    if (HasAuthority())
    {
        ServerBeginCrownChannel_Implementation();
    }
    else
    {
        ServerBeginCrownChannel();
    }
}

void ACalamityCharacter::ServerBeginCrownChannel_Implementation()
{
    if (bEliminated || !GetController())
    {
        return;
    }
    if (ACalamityGameMode* GameMode = GetWorld()->GetAuthGameMode<ACalamityGameMode>())
    {
        GameMode->BeginCrownChannel(GetController());
    }
}

void ACalamityCharacter::RequestCancelCrownChannel()
{
    if (HasAuthority())
    {
        ServerCancelCrownChannel_Implementation();
    }
    else
    {
        ServerCancelCrownChannel();
    }
}

void ACalamityCharacter::ServerCancelCrownChannel_Implementation()
{
    if (ACalamityGameMode* GameMode = GetWorld()->GetAuthGameMode<ACalamityGameMode>())
    {
        GameMode->CancelCrownChannel(GetController());
    }
}

void ACalamityCharacter::RequestPickupDroppedCrown()
{
    if (HasAuthority())
    {
        ServerPickupDroppedCrown_Implementation();
    }
    else
    {
        ServerPickupDroppedCrown();
    }
}

void ACalamityCharacter::ServerPickupDroppedCrown_Implementation()
{
    if (bEliminated || !GetController())
    {
        return;
    }
    if (ACalamityGameMode* GameMode = GetWorld()->GetAuthGameMode<ACalamityGameMode>())
    {
        GameMode->TryPickupDroppedCrown(GetController());
    }
}

void ACalamityCharacter::FireAuthoritative()
{
    if (!HasAuthority() || bEliminated || bIsReloading || !GetWorld() || AmmoInMagazine <= 0)
    {
        return;
    }

    const float Now = GetWorld()->GetTimeSeconds();
    if (Now - LastServerFireTime < FireIntervalSeconds)
    {
        return;
    }
    LastServerFireTime = Now;
    --AmmoInMagazine;

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
    DOREPLIFETIME(ACalamityCharacter, AmmoInMagazine);
    DOREPLIFETIME(ACalamityCharacter, ReserveAmmo);
}
