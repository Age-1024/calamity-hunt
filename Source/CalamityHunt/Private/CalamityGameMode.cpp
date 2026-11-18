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
    if (ACalamityGameState* S = GetCalamityGameState()) { S->CurrentPhase = ECalamityMatchPhase::Scavenging; S->MatchElapsedSeconds = 0.0f; }
}
void ACalamityGameMode::Tick(float Dt)
{
    Super::Tick(Dt);
    ACalamityGameState* S = GetCalamityGameState();
    if (!S || S->CurrentPhase == ECalamityMatchPhase::Finished) return;
    S->MatchElapsedSeconds += Dt;
    if (S->MatchElapsedSeconds >= MatchDurationSeconds) { ResolveTimeout(); return; }
    S->CurrentPhase = S->MatchElapsedSeconds >= BloodMoonStartSeconds ? ECalamityMatchPhase::BloodMoon :
        (S->MatchElapsedSeconds >= CrownContestStartSeconds ? ECalamityMatchPhase::CrownContest : ECalamityMatchPhase::Scavenging);
    UpdateCrownChannel(Dt, S);
    if (bCrownIsDropped && (CrownDroppedElapsedSeconds += Dt) >= CrownResetAfterDropSeconds) ResetDroppedCrown();
    if (S->CurrentPhase == ECalamityMatchPhase::BloodMoon && S->CrownHolder)
    {
        S->CrownHoldProgressSeconds += Dt;
        if (S->CrownHoldProgressSeconds >= CrownHoldToWinSeconds) SetWinner(S->CrownHolder);
    }
}
void ACalamityGameMode::Logout(AController* Exiting)
{
    if (Exiting) { BreakAlliance(Exiting); CancelCrownChannel(Exiting); if (ACalamityGameState* S=GetCalamityGameState()) if (S->CrownHolder==Exiting->PlayerState) DropCrown(Exiting); }
    Super::Logout(Exiting);
}
ACalamityGameState* ACalamityGameMode::GetCalamityGameState() const { return GetGameState<ACalamityGameState>(); }
bool ACalamityGameMode::IsControllerAlive(const AController* C) const
{
    const ACalamityCharacter* Ch = C ? Cast<ACalamityCharacter>(C->GetPawn()) : nullptr;
    return Ch && !Ch->IsEliminated();
}
bool ACalamityGameMode::IsAtAltar(const AController* C) const
{
    return C && C->GetPawn() && FVector::DistSquared(C->GetPawn()->GetActorLocation(), CrownAltarLocation) <= FMath::Square(CrownAltarUseRadius);
}
bool ACalamityGameMode::IsCrownContestOpen(const ACalamityGameState* S) const
{
    return S && (S->CurrentPhase == ECalamityMatchPhase::CrownContest || S->CurrentPhase == ECalamityMatchPhase::BloodMoon);
}
AController* ACalamityGameMode::FindController(const APlayerState* P) const
{
    if (!P || !GetWorld()) return nullptr;
    for (FConstPlayerControllerIterator I=GetWorld()->GetPlayerControllerIterator(); I; ++I) if (APlayerController* PC=I->Get()) if (PC->PlayerState==P) return PC;
    return nullptr;
}
bool ACalamityGameMode::BeginCrownChannel(AController* C)
{
    ACalamityGameState* S=GetCalamityGameState();
    if (!HasAuthority() || !C || !C->PlayerState || !IsControllerAlive(C) || !S || !IsCrownContestOpen(S) || S->CrownHolder || bCrownIsDropped || !IsAtAltar(C)) return false;
    if (S->CrownChannelingPlayer && S->CrownChannelingPlayer!=C->PlayerState) return false;
    S->CrownChannelingPlayer=C->PlayerState; S->CrownChannelProgressSeconds=0.0f; return true;
}
void ACalamityGameMode::CancelCrownChannel(AController* C)
{
    if (!HasAuthority() || !C) return;
    if (ACalamityGameState* S=GetCalamityGameState()) if (S->CrownChannelingPlayer==C->PlayerState) { S->CrownChannelingPlayer=nullptr; S->CrownChannelProgressSeconds=0.0f; }
}
void ACalamityGameMode::UpdateCrownChannel(float Dt, ACalamityGameState* S)
{
    if (!S || !S->CrownChannelingPlayer) return;
    AController* C=FindController(S->CrownChannelingPlayer);
    if (!C || !IsControllerAlive(C) || !IsAtAltar(C) || !IsCrownContestOpen(S) || S->CrownHolder || bCrownIsDropped) { S->CrownChannelingPlayer=nullptr; S->CrownChannelProgressSeconds=0.0f; return; }
    S->CrownChannelProgressSeconds+=Dt;
    if (S->CrownChannelProgressSeconds>=CrownChannelDurationSeconds) { S->CrownHolder=S->CrownChannelingPlayer; S->CrownHoldProgressSeconds=0.0f; S->CrownChannelingPlayer=nullptr; S->CrownChannelProgressSeconds=0.0f; }
}
bool ACalamityGameMode::TryPickupDroppedCrown(AController* C)
{
    if (!HasAuthority() || !C || !C->PlayerState || !IsControllerAlive(C) || !DroppedCrown) return false;
    ACalamityGameState* S=GetCalamityGameState(); const APawn* P=C->GetPawn();
    if (!S || !P || !IsCrownContestOpen(S) || S->CrownHolder || FVector::DistSquared(P->GetActorLocation(),DroppedCrown->GetActorLocation())>FMath::Square(CrownPickupRadius)) return false;
    S->CrownHolder=C->PlayerState; S->CrownHoldProgressSeconds=0.0f; DroppedCrown->Destroy(); DroppedCrown=nullptr; bCrownIsDropped=false; CrownDroppedElapsedSeconds=0.0f; return true;
}
void ACalamityGameMode::DropCrown(AController* C)
{
    if (!HasAuthority() || !C) return;
    ACalamityGameState* S=GetCalamityGameState(); if (!S || S->CrownHolder!=C->PlayerState) return;
    S->CrownHolder=nullptr; S->CrownHoldProgressSeconds=0.0f; S->CrownChannelingPlayer=nullptr; S->CrownChannelProgressSeconds=0.0f;
    if (DroppedCrown) DroppedCrown->Destroy();
    const FVector L=C->GetPawn()?C->GetPawn()->GetActorLocation():CrownAltarLocation;
    FActorSpawnParameters P; P.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    DroppedCrown=GetWorld()->SpawnActor<ACalamityCrownPickup>(ACalamityCrownPickup::StaticClass(),L,FRotator::ZeroRotator,P);
    bCrownIsDropped=DroppedCrown!=nullptr; CrownDroppedElapsedSeconds=0.0f;
}
bool ACalamityGameMode::RequestAllianceOffer(AController* A, APlayerState* B)
{
    if (!HasAuthority() || !A || !A->PlayerState || !B || A->PlayerState==B || !IsControllerAlive(A)) return false;
    ACalamityGameState* S=GetCalamityGameState(); AController* BC=FindController(B);
    if (!S || !BC || !IsControllerAlive(BC) || IsInAlliance(A->PlayerState) || IsInAlliance(B)) return false;
    const APawn* AP=A->GetPawn(); const APawn* BP=BC->GetPawn();
    if (!AP || !BP || FVector::DistSquared(AP->GetActorLocation(),BP->GetActorLocation())>FMath::Square(AllianceOfferDistance)) return false;
    for (const FCalamityAllianceOffer& O:S->AllianceOffers) if (O.Sender==A->PlayerState || O.Recipient==A->PlayerState || O.Sender==B || O.Recipient==B) return false;
    FCalamityAllianceOffer O; O.Sender=A->PlayerState; O.Recipient=B; S->AllianceOffers.Add(O); return true;
}
bool ACalamityGameMode::AcceptAllianceOffer(AController* C)
{
    if (!HasAuthority() || !C || !C->PlayerState || !IsControllerAlive(C)) return false;
    ACalamityGameState* S=GetCalamityGameState(); if (!S || IsInAlliance(C->PlayerState)) return false;
    const int32 I=S->AllianceOffers.IndexOfByPredicate([C](const FCalamityAllianceOffer& O){return O.Recipient==C->PlayerState;});
    if (I==INDEX_NONE) return false;
    const FCalamityAllianceOffer O=S->AllianceOffers[I]; AController* Sender=FindController(O.Sender);
    if (!Sender || !IsControllerAlive(Sender) || IsInAlliance(O.Sender) || !Sender->GetPawn() || !C->GetPawn() || FVector::DistSquared(Sender->GetPawn()->GetActorLocation(),C->GetPawn()->GetActorLocation())>FMath::Square(AllianceOfferDistance)) { S->AllianceOffers.RemoveAt(I); return false; }
    FCalamityAlliancePair P; P.First=O.Sender; P.Second=O.Recipient; S->Alliances.Add(P); S->AllianceOffers.RemoveAt(I); return true;
}
void ACalamityGameMode::BreakAlliance(AController* C)
{
    if (!HasAuthority() || !C || !C->PlayerState) return;
    if (ACalamityGameState* S=GetCalamityGameState()) { const APlayerState* P=C->PlayerState; S->Alliances.RemoveAll([P](const FCalamityAlliancePair& X){return X.First==P||X.Second==P;}); S->AllianceOffers.RemoveAll([P](const FCalamityAllianceOffer& X){return X.Sender==P||X.Recipient==P;}); }
}
bool ACalamityGameMode::IsInAlliance(const APlayerState* P) const
{
    const ACalamityGameState* S=GetCalamityGameState(); return S && S->Alliances.ContainsByPredicate([P](const FCalamityAlliancePair& X){return X.First==P||X.Second==P;});
}
bool ACalamityGameMode::AreAllied(const APlayerState* A, const APlayerState* B) const
{
    if (!A || !B || A==B) return false;
    const ACalamityGameState* S=GetCalamityGameState(); return S && S->Alliances.ContainsByPredicate([A,B](const FCalamityAlliancePair& X){return (X.First==A&&X.Second==B)||(X.First==B&&X.Second==A);});
}
void ACalamityGameMode::HandlePlayerEliminated(AController* E, AController* K)
{
    if (!HasAuthority()) return;
    ACalamityGameState* S=GetCalamityGameState(); if (!S || S->CurrentPhase==ECalamityMatchPhase::Finished) return;
    if (K && K!=E && K->PlayerState && (!E || !AreAllied(K->PlayerState,E->PlayerState))) K->PlayerState->SetScore(K->PlayerState->GetScore()+EliminationScore);
    if (S->CrownHolder && E && S->CrownHolder==E->PlayerState) DropCrown(E);
    CancelCrownChannel(E); BreakAlliance(E);
    APlayerState* Last=nullptr; int32 Count=0;
    for (FConstPlayerControllerIterator I=GetWorld()->GetPlayerControllerIterator(); I; ++I) if (APlayerController* PC=I->Get()) if (IsControllerAlive(PC)) { ++Count; Last=PC->PlayerState; }
    if (Count==1) SetWinner(Last);
}
void ACalamityGameMode::ResetDroppedCrown()
{
    if (DroppedCrown) DroppedCrown->Destroy(); DroppedCrown=nullptr; bCrownIsDropped=false; CrownDroppedElapsedSeconds=0.0f;
    if (ACalamityGameState* S=GetCalamityGameState()) { S->CrownHolder=nullptr; S->CrownHoldProgressSeconds=0.0f; }
}
void ACalamityGameMode::SetWinner(APlayerState* W)
{
    if (!HasAuthority()) return;
    if (ACalamityGameState* S=GetCalamityGameState()) { S->MatchWinner=W; S->CurrentPhase=ECalamityMatchPhase::Finished; S->CrownChannelingPlayer=nullptr; S->CrownChannelProgressSeconds=0.0f; S->AllianceOffers.Reset(); }
}
void ACalamityGameMode::ResolveTimeout()
{
    ACalamityGameState* S=GetCalamityGameState(); if (!S) return;
    if (S->CrownHolder) { SetWinner(S->CrownHolder); return; }
    APlayerState* Best=nullptr; float Score=-TNumericLimits<float>::Max(), Health=-1.0f; int32 Id=MAX_int32;
    for (FConstPlayerControllerIterator I=GetWorld()->GetPlayerControllerIterator(); I; ++I) if (APlayerController* PC=I->Get())
    {
        if (!IsControllerAlive(PC)||!PC->PlayerState) continue;
        const ACalamityCharacter* Ch=Cast<ACalamityCharacter>(PC->GetPawn()); const float H=Ch?Ch->GetHealth():0.0f; const float Sc=PC->PlayerState->GetScore(); const int32 Pid=PC->PlayerState->GetPlayerId();
        if (Sc>Score || (FMath::IsNearlyEqual(Sc,Score)&&(H>Health||(FMath::IsNearlyEqual(H,Health)&&Pid<Id)))) { Best=PC->PlayerState; Score=Sc; Health=H; Id=Pid; }
    }
    SetWinner(Best);
}
