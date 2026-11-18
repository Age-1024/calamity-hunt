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

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Match", meta=(ClampMin="1.0")) float MatchDurationSeconds = 900.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Match", meta=(ClampMin="0.0")) float CrownContestStartSeconds = 480.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Match", meta=(ClampMin="0.0")) float BloodMoonStartSeconds = 660.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Crown", meta=(ClampMin="1.0")) float CrownHoldToWinSeconds = 90.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Crown", meta=(ClampMin="1.0")) float CrownResetAfterDropSeconds = 30.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Crown|Altar") FVector CrownAltarLocation = FVector::ZeroVector;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Crown|Altar", meta=(ClampMin="50.0")) float CrownAltarUseRadius = 250.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Crown|Altar", meta=(ClampMin="0.1")) float CrownChannelDurationSeconds = 5.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Crown|Pickup", meta=(ClampMin="50.0")) float CrownPickupRadius = 180.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Alliance") bool bAllowFriendlyFire = false;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Alliance", meta=(ClampMin="100.0")) float AllianceOfferDistance = 1000.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Scoring", meta=(ClampMin="0.0")) float EliminationScore = 1.0f;

    UFUNCTION(BlueprintCallable, Category="Crown") bool BeginCrownChannel(AController* Controller);
    UFUNCTION(BlueprintCallable, Category="Crown") void CancelCrownChannel(AController* Controller);
    UFUNCTION(BlueprintCallable, Category="Crown") bool TryPickupDroppedCrown(AController* Controller);
    UFUNCTION(BlueprintCallable, Category="Crown") void DropCrown(AController* Controller);
    UFUNCTION(BlueprintCallable, Category="Alliance") bool RequestAllianceOffer(AController* Proposer, APlayerState* Target);
    UFUNCTION(BlueprintCallable, Category="Alliance") bool AcceptAllianceOffer(AController* Accepter);
    UFUNCTION(BlueprintCallable, Category="Alliance") void BreakAlliance(AController* Controller);
    UFUNCTION(BlueprintPure, Category="Alliance") bool AreAllied(const APlayerState* A, const APlayerState* B) const;
    void HandlePlayerEliminated(AController* Eliminated, AController* Killer);

private:
    UPROPERTY() TObjectPtr<ACalamityCrownPickup> DroppedCrown = nullptr;
    float CrownDroppedElapsedSeconds = 0.0f;
    bool bCrownIsDropped = false;
    ACalamityGameState* GetCalamityGameState() const;
    bool IsControllerAlive(const AController* Controller) const;
    bool IsAtAltar(const AController* Controller) const;
    bool IsCrownContestOpen(const ACalamityGameState* State) const;
    bool IsInAlliance(const APlayerState* Player) const;
    AController* FindController(const APlayerState* Player) const;
    void UpdateCrownChannel(float DeltaSeconds, ACalamityGameState* State);
    void ResetDroppedCrown();
    void SetWinner(APlayerState* Winner);
    void ResolveTimeout();
};
