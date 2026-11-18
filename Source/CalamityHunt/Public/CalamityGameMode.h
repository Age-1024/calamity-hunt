// Copyright (c) 2026 Age-1024. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CalamityGameState.h"
#include "CalamityCrownPickup.h"
#include "CalamityGameMode.generated.h"

UCLASS()
class CALAMITYHUNT_API ACalamityGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ACalamityGameMode();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void Logout(AController* Exiting) override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Match|Timing", meta = (ClampMin = "1.0"))
    float MatchDurationSeconds = 900.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Match|Timing", meta = (ClampMin = "0.0"))
    float CrownContestStartSeconds = 480.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Match|Timing", meta = (ClampMin = "0.0"))
    float BloodMoonStartSeconds = 660.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crown|Rules", meta = (ClampMin = "1.0"))
    float CrownHoldToWinSeconds = 90.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crown|Rules", meta = (ClampMin = "1.0"))
    float CrownResetAfterDropSeconds = 30.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crown|Altar")
    FVector CrownAltarLocation = FVector::ZeroVector;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crown|Altar", meta = (ClampMin = "50.0"))
    float CrownAltarUseRadius = 250.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crown|Altar", meta = (ClampMin = "0.1"))
    float CrownChannelDurationSeconds = 5.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crown|Pickup", meta = (ClampMin = "50.0"))
    float CrownPickupRadius = 180.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Alliance|Rules")
    bool bAllowFriendlyFire = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Alliance|Rules", meta = (ClampMin = "100.0"))
    float AllianceOfferDistance = 1000.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scoring", meta = (ClampMin = "0.0"))
    float EliminationScore = 1.0f;

    UFUNCTION(BlueprintCallable, Category = "Crown")
    bool BeginCrownChannel(AController* ClaimingController);

    UFUNCTION(BlueprintCallable, Category = "Crown")
    void CancelCrownChannel(AController* ClaimingController);

    UFUNCTION(BlueprintCallable, Category = "Crown")
    bool TryPickupDroppedCrown(AController* ClaimingController);

    UFUNCTION(BlueprintCallable, Category = "Crown")
    void DropCrown(AController* HolderController);

    UFUNCTION(BlueprintCallable, Category = "Alliance")
    bool RequestAllianceOffer(AController* Proposer, APlayerState* TargetPlayerState);

    UFUNCTION(BlueprintCallable, Category = "Alliance")
    bool AcceptAllianceOffer(AController* Accepter);

    UFUNCTION(BlueprintCallable, Category = "Alliance")
    void BreakAlliance(AController* RequestingController);

    UFUNCTION(BlueprintPure, Category = "Alliance")
    bool AreAllied(const APlayerState* First, const APlayerState* Second) const;

    void HandlePlayerEliminated(AController* EliminatedController, AController* KillerController);

private:
    float CrownDroppedElapsedSeconds = 0.0f;
    bool bCrownIsDropped = false;
    TObjectPtr<ACalamityCrownPickup> DroppedCrown = nullptr;

    ACalamityGameState* GetCalamityGameState() const;
    bool IsControllerAlive(const AController* Controller) const;
    bool IsAtCrownAltar(const AController* Controller) const;
    bool IsCrownContestOpen(const ACalamityGameState* State) const;
    AController* FindControllerForPlayerState(const APlayerState* PlayerState) const;
    bool IsInAlliance(const APlayerState* PlayerState) const;
    void SetWinner(APlayerState* Winner);
    void ResolveTimeout();
    void UpdateCrownChannel(float DeltaSeconds, ACalamityGameState* State);
    void ResetDroppedCrown();
};
