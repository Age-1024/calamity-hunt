// Copyright (c) 2026 Age-1024. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CalamityGameState.generated.h"

UENUM(BlueprintType)
enum class ECalamityMatchPhase : uint8
{
    Scavenging UMETA(DisplayName = "Scavenging"),
    CrownContest UMETA(DisplayName = "Crown Contest"),
    BloodMoon UMETA(DisplayName = "Blood Moon"),
    Finished UMETA(DisplayName = "Finished")
};

UCLASS()
class CALAMITYHUNT_API ACalamityGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Match")
    ECalamityMatchPhase CurrentPhase = ECalamityMatchPhase::Scavenging;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Crown")
    TObjectPtr<APlayerState> CrownHolder = nullptr;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Match")
    TObjectPtr<APlayerState> MatchWinner = nullptr;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Crown")
    float CrownHoldProgressSeconds = 0.0f;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Match")
    float MatchElapsedSeconds = 0.0f;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
