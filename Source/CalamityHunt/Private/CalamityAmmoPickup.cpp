// Copyright (c) 2026 Age-1024. Licensed under the MIT License.
#include "CalamityAmmoPickup.h"
#include "CalamityCharacter.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

ACalamityAmmoPickup::ACalamityAmmoPickup(){bReplicates=true;Trigger=CreateDefaultSubobject<USphereComponent>(TEXT("Trigger"));SetRootComponent(Trigger);Trigger->InitSphereRadius(90.0f);Trigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);Trigger->SetCollisionResponseToAllChannels(ECR_Ignore);Trigger->SetCollisionResponseToChannel(ECC_Pawn,ECR_Overlap);Trigger->OnComponentBeginOverlap.AddDynamic(this,&ACalamityAmmoPickup::OnOverlap);}
void ACalamityAmmoPickup::BeginPlay(){Super::BeginPlay();}
void ACalamityAmmoPickup::OnOverlap(UPrimitiveComponent*,AActor* Other,UPrimitiveComponent*,int32,bool,const FHitResult&){if(!HasAuthority()||!bAvailable)return;ACalamityCharacter* C=Cast<ACalamityCharacter>(Other);if(!C)return;if(C->AddReserveAmmo(AmmoAmount)<=0)return;bAvailable=false;SetActorHiddenInGame(true);SetActorEnableCollision(false);if(RespawnSeconds>0)GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this,[this](){FTimerHandle H;GetWorldTimerManager().SetTimer(H,this,&ACalamityAmmoPickup::RespawnPickup,RespawnSeconds,false);}));}
void ACalamityAmmoPickup::RespawnPickup(){if(!HasAuthority())return;bAvailable=true;SetActorHiddenInGame(false);SetActorEnableCollision(true);Trigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);ForceNetUpdate();}
