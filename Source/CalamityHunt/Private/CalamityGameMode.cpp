// Copyright (c) 2026 Age-1024. Licensed under the MIT License.

#include "CalamityGameMode.h"
#include "CalamityCharacter.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"

ACalamityGameMode::ACalamityGameMode()
{
    PrimaryActorTick.bCanEverTick = true;
    GameStateClass = ACalamityGameState::StaticClass();
    DefaultPawnClass = ACalamityCharacter::StaticClass();
}

void ACalamityGameMode::BeginPlay()
{
    Super::BeginPlay();
    if (ACalamityGameState* State = GetCalamityGameState())
    {
        State->CurrentPhase = ECalamityMatchPhase::Scavenging;
        State->MatchElapsedSeconds = 0.0f;
    }
}

void ACalamityGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    ACalamityGameState* State = GetCalamityGameState();
    if (!State || State->CurrentPhase == ECalamityMatchPhase::Finished)
    {
        return;
    }

    State->MatchElapsedSeconds += DeltaSeconds;
    if (State->MatchElapsedSeconds >= MatchDurationSeconds)
    {
        ResolveTimeout();
        return;
    }

    if (State->MatchElapsedSeconds >= BloodMoonStartSeconds)
    {
        State->CurrentPhase = ECalamityMatchPhase::BloodMoon;
    }
    else if (State->MatchElapsedSeconds >= CrownContestStartSeconds)
    {
        State->CurrentPhase = ECalamityMatchPhase::CrownContest;
    }
    else
    {
        State->CurrentPhase = ECalamityMatchPhase::Scavenging;
    }

    if (bCrownIsDropped)
    {
        CrownDroppedElapsedSeconds += DeltaSeconds;
        if (CrownDroppedElapsedSeconds >= CrownResetAfterDropSeconds)
        {
            // Graybox reset: clearing the holder represents server-side return to the altar.
            // A world pickup actor and altar transform will be added with the interaction/map pass.
            State->CrownHolder = nullptr;
            State->CrownHoldProgressSeconds = 0.0f;
            bCrownIsDropped = false;
            CrownDroppedElapsedSeconds = 0.0f;
        }
    }

    if (State->CurrentPhase == ECalamityMatchPhase::BloodMoon && State->CrownHolder)
    {
        State->CrownHoldProgressSeconds += DeltaSeconds;
        if (State->CrownHoldProgressSeconds >= CrownHoldToWinSeconds)
        {
            SetWinner(State->CrownHolder);
        }
    }
}

bool ACalamityGameMode::TryClaimCrown(AController* ClaimingController)
{
    if (!HasAuthority() || !ClaimingController || !IsControllerAlive(ClaimingController))
    {
        return false;
    }

    ACalamityGameState* State = GetCalamityGameState();
    if (!State || State->CurrentPhase < ECalamityMatchPhase::CrownContest || State->CrownHolder)
    {
        return false;
    }

    State->CrownHolder = ClaimingController->PlayerState;
    State->CrownHoldProgressSeconds = 0.0f;
    bCrownIsDropped = false;
    CrownDroppedElapsedSeconds = 0.0f;
    return State->CrownHolder != nullptr;
}

void ACalamityGameMode::DropCrown(AController* HolderController)
{
    if (!HasAuthority() || !HolderController)
    {
        return;
    }

    ACalamityGameState* State = GetCalamityGameState();
    if (!State || State->CrownHolder != HolderController->PlayerState)
    {
        return;
    }

    State->CrownHolder = nullptr;
    State->CrownHoldProgressSeconds = 0.0f;
    bCrownIsDropped = true;
    CrownDroppedElapsedSeconds = 0.0f;
}

void ACalamityGameMode::HandlePlayerEliminated(AController* EliminatedController)
{
    if (!HasAuthority())
    {
        return;
    }

    ACalamityGameState* State = GetCalamityGameState();
    if (!State || State->CurrentPhase == ECalamityMatchPhase::Finished)
    {
        return;
    }

    if (State->CrownHolder && EliminatedController && State->CrownHolder == EliminatedController->PlayerState)
    {
        DropCrown(EliminatedController);
    }

    APlayerState* LastAlivePlayerState = nullptr;
    int32 AliveCount = 0;
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (PC && IsControllerAlive(PC))
        {
            ++AliveCount;
            LastAlivePlayerState = PC->PlayerState;
        }
    }

    if (AliveCount == 1)
    {
        SetWinner(LastAlivePlayerState);
    }
}

ACalamityGameState* ACalamityGameMode::GetCalamityGameState() const
{
    return GetGameState<ACalamityGameState>();
}

bool ACalamityGameMode::IsControllerAlive(const AController* Controller) const
{
    if (!Controller)
    {
        return false;
    }
    const ACalamityCharacter* Character = Cast<ACalamityCharacter>(Controller->GetPawn());
    return Character && !Character->IsEliminated();
}

void ACalamityGameMode::SetWinner(APlayerState* Winner)
{
    if (!HasAuthority())
    {
        return;
    }
    if (ACalamityGameState* State = GetCalamityGameState())
    {
        State->MatchWinner = Winner;
        State->CurrentPhase = ECalamityMatchPhase::Finished;
    }
}

void ACalamityGameMode::ResolveTimeout()
{
    ACalamityGameState* State = GetCalamityGameState();
    if (!State)
    {
        return;
    }

    if (State->CrownHolder)
    {
        SetWinner(State->CrownHolder);
        return;
    }

    // Temporary, explicit graybox tie-breaker: most remaining health, then lowest PlayerId.
    // Replace with the documented contribution-score rule before public competitive testing.
    APlayerState* BestPlayerState = nullptr;
    float BestHealth = -1.0f;
    int32 BestPlayerId = MAX_int32;
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (!PC || !IsControllerAlive(PC) || !PC->PlayerState)
        {
            continue;
        }
        const ACalamityCharacter* Character = Cast<ACalamityCharacter>(PC->GetPawn());
        const float Health = Character ? Character->GetHealth() : 0.0f;
        const int32 PlayerId = PC->PlayerState->GetPlayerId();
        if (Health > BestHealth || (FMath::IsNearlyEqual(Health, BestHealth) && PlayerId < BestPlayerId))
        {
            BestHealth = Health;
            BestPlayerId = PlayerId;
            BestPlayerState = PC->PlayerState;
        }
    }
    SetWinner(BestPlayerState);
}
