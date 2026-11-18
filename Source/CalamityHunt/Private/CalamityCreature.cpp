// Copyright (c) 2026 Age-1024. Licensed under the MIT License.

#include "CalamityCreature.h"
#include "CalamityCharacter.h"
#include "CalamityGameMode.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

ACalamityCreature::ACalamityCreature()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    SetReplicateMovement(true);
    Health = MaxHealth;
}

void ACalamityCreature::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!HasAuthority() || bIsDead || !GetWorld()) return;

    AttackCooldown = FMath::Max(0.0f, AttackCooldown - DeltaSeconds);
    AActor* Target = FindTarget();
    if (!Target) return;

    const FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
    const float Distance = ToTarget.Size();
    if (Distance > AttackRange)
    {
        AddMovementInput(ToTarget.GetSafeNormal2D(), 1.0f);
        return;
    }

    if (AttackCooldown <= 0.0f)
    {
        AttackCooldown = FMath::Max(0.1f, AttackInterval);
        UGameplayStatics::ApplyDamage(Target, AttackDamage, GetController(), this, nullptr);
    }
}

AActor* ACalamityCreature::FindTarget() const
{
    if (!GetWorld()) return nullptr;

    AActor* BestTarget = nullptr;
    float BestDistanceSquared = TNumericLimits<float>::Max();
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        ACalamityCharacter* Character = PC ? Cast<ACalamityCharacter>(PC->GetPawn()) : nullptr;
        if (!Character || Character->IsEliminated()) continue;
        if (TamedBy && PC->PlayerState == TamedBy) continue;

        if (TamedBy)
        {
            const ACalamityGameMode* GM = GetWorld()->GetAuthGameMode<ACalamityGameMode>();
            if (GM && GM->AreAllied(TamedBy, PC->PlayerState)) continue;
        }

        const float DistanceSquared = FVector::DistSquared(GetActorLocation(), Character->GetActorLocation());
        if (DistanceSquared < BestDistanceSquared)
        {
            BestDistanceSquared = DistanceSquared;
            BestTarget = Character;
        }
    }
    return BestTarget;
}

bool ACalamityCreature::TryTame(AController* Tamer)
{
    if (!HasAuthority() || !Tamer || !Tamer->PlayerState || !Tamer->GetPawn() || bIsDead || TamedBy)
    {
        return false;
    }
    const float RequiredHealth = MaxHealth * FMath::Clamp(TameHealthFraction, 0.0f, 1.0f);
    if (Health > RequiredHealth || FVector::DistSquared(GetActorLocation(), Tamer->GetPawn()->GetActorLocation()) > FMath::Square(250.0f))
    {
        return false;
    }
    TamedBy = Tamer->PlayerState;
    return true;
}

float ACalamityCreature::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if (!HasAuthority() || bIsDead || DamageAmount <= 0.0f) return 0.0f;
    Health = FMath::Max(0.0f, Health - DamageAmount);
    if (Health <= 0.0f) HandleDeath();
    return DamageAmount;
}

void ACalamityCreature::HandleDeath()
{
    if (!HasAuthority() || bIsDead) return;
    bIsDead = true;
    SetActorEnableCollision(false);
    SetLifeSpan(2.0f);
}

void ACalamityCreature::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ACalamityCreature, Health);
    DOREPLIFETIME(ACalamityCreature, TamedBy);
    DOREPLIFETIME(ACalamityCreature, bIsDead);
}
