// Copyright (c) 2026 Age-1024. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CalamityCrownPickup.generated.h"

class USphereComponent;
class UTextRenderComponent;

UCLASS()
class CALAMITYHUNT_API ACalamityCrownPickup : public AActor
{
    GENERATED_BODY()

public:
    ACalamityCrownPickup();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crown")
    TObjectPtr<USphereComponent> Trigger;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crown")
    TObjectPtr<UTextRenderComponent> Label;
};
