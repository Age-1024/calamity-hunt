// Copyright (c) 2026 Age-1024. Licensed under the MIT License.

#include "CalamityCharacter.h"
#include "CalamityCreature.h"
#include "CalamityGameMode.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

ACalamityCharacter::ACalamityCharacter()
{
    bReplicates = true; SetReplicateMovement(true);
    Health = MaxHealth; AmmoInMagazine = MagazineSize; ReserveAmmo = InitialReserveAmmo;
}
void ACalamityCharacter::RequestFire() { if (!bEliminated) { if (HasAuthority()) FireAuthoritative(); else ServerFire(); } }
void ACalamityCharacter::ServerFire_Implementation() { FireAuthoritative(); }
void ACalamityCharacter::RequestReload() { if (HasAuthority()) ReloadAuthoritative(); else ServerReload(); }
void ACalamityCharacter::ServerReload_Implementation() { ReloadAuthoritative(); }

void ACalamityCharacter::ReloadAuthoritative()
{
    if (!HasAuthority() || bEliminated || bIsReloading || AmmoInMagazine >= MagazineSize || ReserveAmmo <= 0 || !GetWorld()) return;
    bIsReloading = true;
    FTimerHandle Timer;
    TWeakObjectPtr<ACalamityCharacter> WeakThis(this);
    GetWorldTimerManager().SetTimer(Timer, [WeakThis]()
    {
        ACalamityCharacter* Self = WeakThis.Get();
        if (!Self || !Self->HasAuthority()) return;
        Self->bIsReloading = false;
        if (Self->bEliminated) return;
        const int32 Loaded = FMath::Min(Self->MagazineSize - Self->AmmoInMagazine, Self->ReserveAmmo);
        Self->AmmoInMagazine += Loaded; Self->ReserveAmmo -= Loaded;
    }, FMath::Max(0.1f, ReloadDurationSeconds), false);
}

#define CALAMITY_FORWARD_REQUEST(RequestName, ServerName) \
void ACalamityCharacter::RequestName() { if (HasAuthority()) ServerName##_Implementation(); else ServerName(); }
CALAMITY_FORWARD_REQUEST(RequestBeginCrownChannel, ServerBeginCrownChannel)
CALAMITY_FORWARD_REQUEST(RequestCancelCrownChannel, ServerCancelCrownChannel)
CALAMITY_FORWARD_REQUEST(RequestPickupDroppedCrown, ServerPickupDroppedCrown)
CALAMITY_FORWARD_REQUEST(RequestAcceptAlliance, ServerAcceptAlliance)
CALAMITY_FORWARD_REQUEST(RequestBreakAlliance, ServerBreakAlliance)
#undef CALAMITY_FORWARD_REQUEST

void ACalamityCharacter::ServerBeginCrownChannel_Implementation()
{ if (!bEliminated && GetController()) if (auto* GM = GetWorld()->GetAuthGameMode<ACalamityGameMode>()) GM->BeginCrownChannel(GetController()); }
void ACalamityCharacter::ServerCancelCrownChannel_Implementation()
{ if (GetController()) if (auto* GM = GetWorld()->GetAuthGameMode<ACalamityGameMode>()) GM->CancelCrownChannel(GetController()); }
void ACalamityCharacter::ServerPickupDroppedCrown_Implementation()
{ if (!bEliminated && GetController()) if (auto* GM = GetWorld()->GetAuthGameMode<ACalamityGameMode>()) GM->TryPickupDroppedCrown(GetController()); }
void ACalamityCharacter::RequestAllianceOffer(APlayerState* Target) { if (HasAuthority()) ServerRequestAllianceOffer_Implementation(Target); else ServerRequestAllianceOffer(Target); }
void ACalamityCharacter::ServerRequestAllianceOffer_Implementation(APlayerState* Target)
{ if (!bEliminated && GetController()) if (auto* GM = GetWorld()->GetAuthGameMode<ACalamityGameMode>()) GM->RequestAllianceOffer(GetController(), Target); }
void ACalamityCharacter::ServerAcceptAlliance_Implementation()
{ if (!bEliminated && GetController()) if (auto* GM = GetWorld()->GetAuthGameMode<ACalamityGameMode>()) GM->AcceptAllianceOffer(GetController()); }
void ACalamityCharacter::ServerBreakAlliance_Implementation()
{ if (GetController()) if (auto* GM = GetWorld()->GetAuthGameMode<ACalamityGameMode>()) GM->BreakAlliance(GetController()); }
void ACalamityCharacter::RequestTameCreature(ACalamityCreature* Creature)
{ if (HasAuthority()) ServerTameCreature_Implementation(Creature); else ServerTameCreature(Creature); }
void ACalamityCharacter::ServerTameCreature_Implementation(ACalamityCreature* Creature)
{
    if (!HasAuthority() || bEliminated || TamingItems <= 0 || !IsValid(Creature) || !GetController()) return;
    if (FVector::DistSquared(GetActorLocation(), Creature->GetActorLocation()) > FMath::Square(250.0f)) return;
    if (Creature->TryTame(GetController())) --TamingItems;
}

void ACalamityCharacter::FireAuthoritative()
{
    if (!HasAuthority() || bEliminated || bIsReloading || !GetWorld() || AmmoInMagazine <= 0) return;
    const float Now = GetWorld()->GetTimeSeconds();
    if (Now - LastServerFireTime < FMath::Max(0.1f, FireIntervalSeconds)) return;
    LastServerFireTime = Now; --AmmoInMagazine;
    const FVector Start = GetPawnViewLocation();
    const FVector Direction = GetBaseAimRotation().Vector();
    FCollisionQueryParams Params(SCENE_QUERY_STAT(CalamityHitscan), true, this); Params.AddIgnoredActor(this);
    FHitResult Hit;
    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, Start + Direction * WeaponRange, ECC_Visibility, Params) && Hit.GetActor())
        UGameplayStatics::ApplyPointDamage(Hit.GetActor(), BaseWeaponDamage, Direction, Hit, GetController(), this, nullptr);
}

float ACalamityCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if (!HasAuthority() || bEliminated || DamageAmount <= 0.0f || !GetWorld()) return 0.0f;
    if (EventInstigator && GetController())
        if (const auto* GM = GetWorld()->GetAuthGameMode<ACalamityGameMode>(); GM && !GM->bAllowFriendlyFire && GM->AreAllied(EventInstigator->PlayerState, GetController()->PlayerState)) return 0.0f;
    Health = FMath::Max(0.0f, Health - DamageAmount);
    if (Health <= 0.0f) Eliminate(EventInstigator);
    return FMath::Min(Health + DamageAmount, DamageAmount);
}
void ACalamityCharacter::Eliminate(AController* Killer)
{
    if (!HasAuthority() || bEliminated) return;
    bEliminated = true; SetActorEnableCollision(false);
    if (auto* GM = GetWorld()->GetAuthGameMode<ACalamityGameMode>()) GM->HandlePlayerEliminated(GetController(), Killer);
}
void ACalamityCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const
{
    Super::GetLifetimeReplicatedProps(Out);
    DOREPLIFETIME(ACalamityCharacter, MaxHealth); DOREPLIFETIME(ACalamityCharacter, Health);
    DOREPLIFETIME(ACalamityCharacter, bEliminated); DOREPLIFETIME(ACalamityCharacter, AmmoInMagazine);
    DOREPLIFETIME(ACalamityCharacter, ReserveAmmo); DOREPLIFETIME(ACalamityCharacter, TamingItems);
}
