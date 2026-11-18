// Copyright (c) 2026 Age-1024. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CalamityGameState.h"
#include "CalamityGameMode.generated.h"

UCLASS()
class CALAMITYHUNT_API ACalamityGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ACalamityGameMode();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

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

    UFUNCTION(BlueprintCallable, Category = "Crown")
    bool TryClaimCrown(AController* ClaimingController);

    UFUNCTION(BlueprintCallable, Category = "Crown")
    void DropCrown(AController* HolderController);

    void HandlePlayerEliminated(AController* EliminatedController);

private:
    float CrownDroppedElapsedSeconds = 0.0f;
    bool bCrownIsDropped = false;

    ACalamityGameState* GetCalamityGameState() const;
    bool IsControllerAlive(const AController* Controller) const;
    void SetWinner(APlayerState* Winner);
    void ResolveTimeout();
};
