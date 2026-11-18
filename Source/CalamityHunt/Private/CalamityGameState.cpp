// Copyright (c) 2026 Age-1024. Licensed under the MIT License.

#include "CalamityGameState.h"
#include "Net/UnrealNetwork.h"

void ACalamityGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ACalamityGameState, CurrentPhase);
    DOREPLIFETIME(ACalamityGameState, CrownHolder);
    DOREPLIFETIME(ACalamityGameState, CrownChannelingPlayer);
    DOREPLIFETIME(ACalamityGameState, MatchWinner);
    DOREPLIFETIME(ACalamityGameState, CrownHoldProgressSeconds);
    DOREPLIFETIME(ACalamityGameState, CrownChannelProgressSeconds);
    DOREPLIFETIME(ACalamityGameState, MatchElapsedSeconds);
    DOREPLIFETIME(ACalamityGameState, Alliances);
    DOREPLIFETIME(ACalamityGameState, AllianceOffers);
}
