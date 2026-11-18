// Copyright (c) 2026 Age-1024. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CalamityGameState.generated.h"

class APlayerState;

UENUM(BlueprintType)
enum class ECalamityMatchPhase : uint8
{
    Scavenging UMETA(DisplayName = "Scavenging"),
    CrownContest UMETA(DisplayName = "Crown Contest"),
    BloodMoon UMETA(DisplayName = "Blood Moon"),
    Finished UMETA(DisplayName = "Finished")
};

USTRUCT(BlueprintType)
struct FCalamityAlliancePair
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    TObjectPtr<APlayerState> First = nullptr;

    UPROPERTY(BlueprintReadOnly)
    TObjectPtr<APlayerState> Second = nullptr;
};

USTRUCT(BlueprintType)
struct FCalamityAllianceOffer
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    TObjectPtr<APlayerState> Sender = nullptr;

    UPROPERTY(BlueprintReadOnly)
    TObjectPtr<APlayerState> Recipient = nullptr;
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

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Crown")
    TObjectPtr<APlayerState> CrownChannelingPlayer = nullptr;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Match")
    TObjectPtr<APlayerState> MatchWinner = nullptr;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Crown")
    float CrownHoldProgressSeconds = 0.0f;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Crown")
    float CrownChannelProgressSeconds = 0.0f;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Match")
    float MatchElapsedSeconds = 0.0f;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Alliance")
    TArray<FCalamityAlliancePair> Alliances;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Alliance")
    TArray<FCalamityAllianceOffer> AllianceOffers;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
