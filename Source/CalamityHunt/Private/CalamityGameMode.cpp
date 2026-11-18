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

    UpdateCrownChannel(DeltaSeconds, State);
    if (bCrownIsDropped)
    {
        CrownDroppedElapsedSeconds += DeltaSeconds;
        if (CrownDroppedElapsedSeconds >= CrownResetAfterDropSeconds)
        {
            ResetDroppedCrown();
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

void ACalamityGameMode::Logout(AController* Exiting)
{
    BreakAlliance(Exiting);
    CancelCrownChannel(Exiting);
    if (ACalamityGameState* State = GetCalamityGameState())
    {
        if (Exiting && State->CrownHolder == Exiting->PlayerState)
        {
            DropCrown(Exiting);
        }
    }
    Super::Logout(Exiting);
}

bool ACalamityGameMode::BeginCrownChannel(AController* ClaimingController)
{
    if (!HasAuthority() || !ClaimingController || !ClaimingController->PlayerState || !IsControllerAlive(ClaimingController))
    {
        return false;
    }

    ACalamityGameState* State = GetCalamityGameState();
    if (!State || !IsCrownContestOpen(State) || State->CrownHolder || bCrownIsDropped ||
        !IsAtCrownAltar(ClaimingController))
    {
        return false;
    }
    if (State->CrownChannelingPlayer && State->CrownChannelingPlayer != ClaimingController->PlayerState)
    {
        return false;
    }

    State->CrownChannelingPlayer = ClaimingController->PlayerState;
    State->CrownChannelProgressSeconds = 0.0f;
    return true;
}

void ACalamityGameMode::CancelCrownChannel(AController* ClaimingController)
{
    if (!HasAuthority() || !ClaimingController)
    {
        return;
    }
    if (ACalamityGameState* State = GetCalamityGameState())
    {
        if (State->CrownChannelingPlayer == ClaimingController->PlayerState)
        {
            State->CrownChannelingPlayer = nullptr;
            State->CrownChannelProgressSeconds = 0.0f;
        }
    }
}

void ACalamityGameMode::UpdateCrownChannel(float DeltaSeconds, ACalamityGameState* State)
{
    if (!State || !State->CrownChannelingPlayer)
    {
        return;
    }

    AController* ChannelingController = FindControllerForPlayerState(State->CrownChannelingPlayer);
    if (!ChannelingController || !IsControllerAlive(ChannelingController) || !IsAtCrownAltar(ChannelingController) ||
        !IsCrownContestOpen(State) || State->CrownHolder || bCrownIsDropped)
    {
        State->CrownChannelingPlayer = nullptr;
        State->CrownChannelProgressSeconds = 0.0f;
        return;
    }

    State->CrownChannelProgressSeconds += DeltaSeconds;
    if (State->CrownChannelProgressSeconds >= CrownChannelDurationSeconds)
    {
        State->CrownHolder = State->CrownChannelingPlayer;
        State->CrownHoldProgressSeconds = 0.0f;
        State->CrownChannelingPlayer = nullptr;
        State->CrownChannelProgressSeconds = 0.0f;
    }
}

bool ACalamityGameMode::TryPickupDroppedCrown(AController* ClaimingController)
{
    if (!HasAuthority() || !ClaimingController || !ClaimingController->PlayerState ||
        !IsControllerAlive(ClaimingController) || !DroppedCrown)
    {
        return false;
    }

    ACalamityGameState* State = GetCalamityGameState();
    APawn* Pawn = ClaimingController->GetPawn();
    if (!State || !Pawn || !IsCrownContestOpen(State) || State->CrownHolder ||
        FVector::DistSquared(Pawn->GetActorLocation(), DroppedCrown->GetActorLocation()) > FMath::Square(CrownPickupRadius))
    {
        return false;
    }

    State->CrownHolder = ClaimingController->PlayerState;
    State->CrownHoldProgressSeconds = 0.0f;
    State->CrownChannelingPlayer = nullptr;
    State->CrownChannelProgressSeconds = 0.0f;
    DroppedCrown->Destroy();
    DroppedCrown = nullptr;
    bCrownIsDropped = false;
    CrownDroppedElapsedSeconds = 0.0f;
    return true;
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
    State->CrownChannelingPlayer = nullptr;
    State->CrownChannelProgressSeconds = 0.0f;
    if (DroppedCrown)
    {
        DroppedCrown->Destroy();
        DroppedCrown = nullptr;
    }

    const APawn* Pawn = HolderController->GetPawn();
    const FVector DropLocation = Pawn ? Pawn->GetActorLocation() : CrownAltarLocation;
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    DroppedCrown = GetWorld()->SpawnActor<ACalamityCrownPickup>(
        ACalamityCrownPickup::StaticClass(), DropLocation, FRotator::ZeroRotator, SpawnParams);
    bCrownIsDropped = DroppedCrown != nullptr;
    CrownDroppedElapsedSeconds = 0.0f;
}

bool ACalamityGameMode::RequestAllianceOffer(AController* Proposer, APlayerState* TargetPlayerState)
{
    if (!HasAuthority() || !Proposer || !Proposer->PlayerState || !TargetPlayerState ||
        Proposer->PlayerState == TargetPlayerState || !IsControllerAlive(Proposer))
    {
        return false;
    }

    ACalamityGameState* State = GetCalamityGameState();
    AController* TargetController = FindControllerForPlayerState(TargetPlayerState);
    if (!State || State->CurrentPhase == ECalamityMatchPhase::Finished || !TargetController ||
        !IsControllerAlive(TargetController) || IsInAlliance(Proposer->PlayerState) || IsInAlliance(TargetPlayerState))
    {
        return false;
    }

    const APawn* ProposerPawn = Proposer->GetPawn();
    const APawn* TargetPawn = TargetController->GetPawn();
    if (!ProposerPawn || !TargetPawn || FVector::DistSquared(ProposerPawn->GetActorLocation(), TargetPawn->GetActorLocation()) >
        FMath::Square(AllianceOfferDistance))
    {
        return false;
    }

    for (const FCalamityAllianceOffer& Offer : State->AllianceOffers)
    {
        if (Offer.Sender == Proposer->PlayerState || Offer.Recipient == Proposer->PlayerState ||
            Offer.Sender == TargetPlayerState || Offer.Recipient == TargetPlayerState)
        {
            return false;
        }
    }

    FCalamityAllianceOffer NewOffer;
    NewOffer.Sender = Proposer->PlayerState;
    NewOffer.Recipient = TargetPlayerState;
    State->AllianceOffers.Add(NewOffer);
    return true;
}

bool ACalamityGameMode::AcceptAllianceOffer(AController* Accepter)
{
    if (!HasAuthority() || !Accepter || !Accepter->PlayerState || !IsControllerAlive(Accepter))
    {
        return false;
    }

    ACalamityGameState* State = GetCalamityGameState();
    if (!State || State->CurrentPhase == ECalamityMatchPhase::Finished || IsInAlliance(Accepter->PlayerState))
    {
        return false;
    }

    const int32 OfferIndex = State->AllianceOffers.IndexOfByPredicate(
        [Accepter](const FCalamityAllianceOffer& Offer) { return Offer.Recipient == Accepter->PlayerState; });
    if (OfferIndex == INDEX_NONE)
    {
        return false;
    }

    const FCalamityAllianceOffer Offer = State->AllianceOffers[OfferIndex];
    AController* SenderController = FindControllerForPlayerState(Offer.Sender);
    const APawn* SenderPawn = SenderController ? SenderController->GetPawn() : nullptr;
    const APawn* AccepterPawn = Accepter->GetPawn();
    if (!SenderController || !IsControllerAlive(SenderController) || IsInAlliance(Offer.Sender) ||
        !SenderPawn || !AccepterPawn || FVector::DistSquared(SenderPawn->GetActorLocation(), AccepterPawn->GetActorLocation()) >
        FMath::Square(AllianceOfferDistance))
    {
        State->AllianceOffers.RemoveAt(OfferIndex);
        return false;
    }

    FCalamityAlliancePair NewAlliance;
    NewAlliance.First = Offer.Sender;
    NewAlliance.Second = Offer.Recipient;
    State->Alliances.Add(NewAlliance);
    State->AllianceOffers.RemoveAt(OfferIndex);
    return true;
}

void ACalamityGameMode::BreakAlliance(AController* RequestingController)
{
    if (!HasAuthority() || !RequestingController || !RequestingController->PlayerState)
    {
        return;
    }
    if (ACalamityGameState* State = GetCalamityGameState())
    {
        const APlayerState* PlayerState = RequestingController->PlayerState;
        State->Alliances.RemoveAll([PlayerState](const FCalamityAlliancePair& Pair)
        {
            return Pair.First == PlayerState || Pair.Second == PlayerState;
        });
        State->AllianceOffers.RemoveAll([PlayerState](const FCalamityAllianceOffer& Offer)
        {
            return Offer.Sender == PlayerState || Offer.Recipient == PlayerState;
        });
    }
}

bool ACalamityGameMode::AreAllied(const APlayerState* First, const APlayerState* Second) const
{
    if (!First || !Second || First == Second)
    {
        return false;
    }
    const ACalamityGameState* State = GetCalamityGameState();
    return State && State->Alliances.ContainsByPredicate([First, Second](const FCalamityAlliancePair& Pair)
    {
        return (Pair.First == First && Pair.Second == Second) || (Pair.First == Second && Pair.Second == First);
    });
}

void ACalamityGameMode::HandlePlayerEliminated(AController* EliminatedController, AController* KillerController)
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

    if (KillerController && KillerController != EliminatedController && KillerController->PlayerState &&
        (!EliminatedController || !AreAllied(KillerController->PlayerState, EliminatedController->PlayerState)))
    {
        KillerController->PlayerState->SetScore(KillerController->PlayerState->GetScore() + EliminationScore);
    }

    if (State->CrownHolder && EliminatedController && State->CrownHolder == EliminatedController->PlayerState)
    {
        DropCrown(EliminatedController);
    }
    CancelCrownChannel(EliminatedController);
    BreakAlliance(EliminatedController);

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

bool ACalamityGameMode::IsAtCrownAltar(const AController* Controller) const
{
    return Controller && Controller->GetPawn() &&
        FVector::DistSquared(Controller->GetPawn()->GetActorLocation(), CrownAltarLocation) <= FMath::Square(CrownAltarUseRadius);
}

bool ACalamityGameMode::IsCrownContestOpen(const ACalamityGameState* State) const
{
    return State && (State->CurrentPhase == ECalamityMatchPhase::CrownContest ||
        State->CurrentPhase == ECalamityMatchPhase::BloodMoon);
}

AController* ACalamityGameMode::FindControllerForPlayerState(const APlayerState* PlayerState) const
{
    if (!PlayerState || !GetWorld())
    {
        return nullptr;
    }
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (PC && PC->PlayerState == PlayerState)
        {
            return PC;
        }
    }
    return nullptr;
}

bool ACalamityGameMode::IsInAlliance(const APlayerState* PlayerState) const
{
    const ACalamityGameState* State = GetCalamityGameState();
    return State && State->Alliances.ContainsByPredicate([PlayerState](const FCalamityAlliancePair& Pair)
    {
        return Pair.First == PlayerState || Pair.Second == PlayerState;
    });
}

void ACalamityGameMode::ResetDroppedCrown()
{
    if (DroppedCrown)
    {
        DroppedCrown->Destroy();
        DroppedCrown = nullptr;
    }
    bCrownIsDropped = false;
    CrownDroppedElapsedSeconds = 0.0f;
    if (ACalamityGameState* State = GetCalamityGameState())
    {
        State->CrownHolder = nullptr;
        State->CrownHoldProgressSeconds = 0.0f;
    }
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
        State->CrownChannelingPlayer = nullptr;
        State->CrownChannelProgressSeconds = 0.0f;
        State->AllianceOffers.Reset();
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

    APlayerState* BestPlayerState = nullptr;
    float BestScore = -TNumericLimits<float>::Max();
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
        const float Score = PC->PlayerState->GetScore();
        const int32 PlayerId = PC->PlayerState->GetPlayerId();
        if (Score > BestScore || (FMath::IsNearlyEqual(Score, BestScore) &&
            (Health > BestHealth || (FMath::IsNearlyEqual(Health, BestHealth) && PlayerId < BestPlayerId))))
        {
            BestScore = Score;
            BestHealth = Health;
            BestPlayerId = PlayerId;
            BestPlayerState = PC->PlayerState;
        }
    }
    SetWinner(BestPlayerState);
}
