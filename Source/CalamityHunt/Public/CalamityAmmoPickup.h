// Copyright (c) 2026 Age-1024. Licensed under the MIT License.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CalamityAmmoPickup.generated.h"
class USphereComponent;
UCLASS()
class CALAMITYHUNT_API ACalamityAmmoPickup : public AActor
{
    GENERATED_BODY()
public:
    ACalamityAmmoPickup();
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Pickup") TObjectPtr<USphereComponent> Trigger;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pickup", meta=(ClampMin="1")) int32 AmmoAmount=24;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pickup", meta=(ClampMin="0.1")) float RespawnSeconds=20.0f;
protected:
    virtual void BeginPlay() override;
private:
    UFUNCTION() void OnOverlap(UPrimitiveComponent* Overlapped, AActor* Other, UPrimitiveComponent* OtherComp, int32 BodyIndex, bool bFromSweep, const FHitResult& Sweep);
    void RespawnPickup();
    bool bAvailable=true;
};
