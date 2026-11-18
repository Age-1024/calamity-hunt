// Copyright (c) 2026 Age-1024. Licensed under the MIT License.

#include "CalamityCharacter.h"
#include "CalamityGameMode.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerState.h"
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
    if (bEliminated) return;
    if (HasAuthority()) FireAuthoritative(); else ServerFire();
}

void ACalamityCharacter::ServerFire_Implementation() { FireAuthoritative(); }

void ACalamityCharacter::RequestReload()
{
    if (HasAuthority()) ReloadAuthoritative(); else ServerReload();
}

void ACalamityCharacter::ServerReload_Implementation() { ReloadAuthoritative(); }

void ACalamityCharacter::ReloadAuthoritative()
{
    if (!HasAuthority() || bEliminated || bIsReloading || AmmoInMagazine >= MagazineSize || ReserveAmmo <= 0 || !GetWorld())
    {
        return;
    }

    bIsReloading = true;
    FTimerHandle ReloadTimer;
    TWeakObjectPtr<ACalamityCharacter> WeakThis(this);
    GetWorldTimerManager().SetTimer(ReloadTimer, [WeakThis]()
    {
        ACalamityCharacter* Character = WeakThis.Get();
        if (!Character || !Character->HasAuthority()) return;
        Character->bIsReloading = false;
        if (Character->bEliminated) return;
        const int32 Loaded = FMath::Min(Character->MagazineSize - Character->AmmoInMagazine, Character->ReserveAmmo);
        Character->AmmoInMagazine += Loaded;
        Character->ReserveAmmo -= Loaded;
    }, FMath::Max(0.1f, ReloadDurationSeconds), false);
}

void ACalamityCharacter::RequestBeginCrownChannel()
{
    if (HasAuthority()) ServerBeginCrownChannel_Implementation(); else ServerBeginCrownChannel();
}

void ACalamityCharacter::ServerBeginCrownChannel_Implementation()
{
    if (bEliminated || !GetController()) return;
    if (ACalamityGameMode* GM = GetWorld()->GetAuthGameMode<ACalamityGameMode>()) GM->BeginCrownChannel(GetController());
}

void ACalamityCharacter::RequestCancelCrownChannel()
{
    if (HasAuthority()) ServerCancelCrownChannel_Implementation(); else ServerCancelCrownChannel();
}

void ACalamityCharacter::ServerCancelCrownChannel_Implementation()
{
    if (ACalamityGameMode* GM = GetWorld()->GetAuthGameMode<ACalamityGameMode>()) GM->CancelCrownChannel(GetController());
}

void ACalamityCharacter::RequestPickupDroppedCrown()
{
    if (HasAuthority()) ServerPickupDroppedCrown_Implementation(); else ServerPickupDroppedCrown();
}

void ACalamityCharacter::ServerPickupDroppedCrown_Implementation()
{
    if (bEliminated || !GetController()) return;
    if (ACalamityGameMode* GM = GetWorld()->GetAuthGameMode<ACalamityGameMode>()) GM->TryPickupDroppedCrown(GetController());
}

void ACalamityCharacter::RequestAllianceOffer(APlayerState* TargetPlayerState)
{
    if (HasAuthority()) ServerRequestAllianceOffer_Implementation(TargetPlayerState);
    else ServerRequestAllianceOffer(TargetPlayerState);
}

void ACalamityCharacter::ServerRequestAllianceOffer_Implementation(APlayerState* TargetPlayerState)
{
    if (bEliminated || !GetController()) return;
    if (ACalamityGameMode* GM = GetWorld()->GetAuthGameMode<ACalamityGameMode>()) GM->RequestAllianceOffer(GetController(), TargetPlayerState);
}

void ACalamityCharacter::RequestAcceptAlliance()
{
    if (HasAuthority()) ServerAcceptAlliance_Implementation(); else ServerAcceptAlliance();
}

void ACalamityCharacter::ServerAcceptAlliance_Implementation()
{
    if (bEliminated || !GetController()) return;
    if (ACalamityGameMode* GM = GetWorld()->GetAuthGameMode<ACalamityGameMode>()) GM->AcceptAllianceOffer(GetController());
}

void ACalamityCharacter::RequestBreakAlliance()
{
    if (HasAuthority()) ServerBreakAlliance_Implementation(); else ServerBreakAlliance();
}

void ACalamityCharacter::ServerBreakAlliance_Implementation()
{
    if (ACalamityGameMode* GM = GetWorld()->GetAuthGameMode<ACalamityGameMode>()) GM->BreakAlliance(GetController());
}

void ACalamityCharacter::FireAuthoritative()
{
    if (!HasAuthority() || bEliminated || bIsReloading || !GetWorld() || AmmoInMagazine <= 0) return;
    const float Now = GetWorld()->GetTimeSeconds();
    if (Now - LastServerFireTime < FMath::Max(0.1f, FireIntervalSeconds)) return;
    LastServerFireTime = Now;
    --AmmoInMagazine;

    const FVector Start = GetPawnViewLocation();
    const FVector Direction = GetBaseAimRotation().Vector();
    const FVector End = Start + Direction * WeaponRange;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(CalamityHitscan), true, this);
    Params.AddIgnoredActor(this);
    FHitResult Hit;
    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params) && Hit.GetActor())
    {
        UGameplayStatics::ApplyPointDamage(Hit.GetActor(), BaseWeaponDamage, Direction, Hit, GetController(), this, nullptr);
    }
}

float ACalamityCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if (!HasAuthority() || bEliminated || DamageAmount <= 0.0f) return 0.0f;
    if (!GetWorld()) return 0.0f;

    if (!EventInstigator || !GetController())
    {
        // Environmental damage is allowed; it has no alliance relationship.
    }
    else if (const ACalamityGameMode* GM = GetWorld()->GetAuthGameMode<ACalamityGameMode>())
    {
        if (!GM->bAllowFriendlyFire && GM->AreAllied(EventInstigator->PlayerState, GetController()->PlayerState)) return 0.0f;
    }

    const float AppliedDamage = FMath::Min(Health, DamageAmount);
    Health = FMath::Max(0.0f, Health - AppliedDamage);
    if (Health <= 0.0f) Eliminate(EventInstigator);
    return AppliedDamage;
}

void ACalamityCharacter::Eliminate(AController* KillerController)
{
    if (!HasAuthority() || bEliminated) return;
    bEliminated = true;
    SetActorEnableCollision(false);
    if (ACalamityGameMode* GM = GetWorld()->GetAuthGameMode<ACalamityGameMode>())
    {
        GM->HandlePlayerEliminated(GetController(), KillerController);
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
