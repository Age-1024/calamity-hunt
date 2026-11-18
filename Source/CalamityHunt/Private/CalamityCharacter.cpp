// Copyright (c) 2026 Age-1024. Licensed under the MIT License.
#include "CalamityCharacter.h"
#include "CalamityCreature.h"
#include "CalamityGameMode.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

ACalamityCharacter::ACalamityCharacter(){bReplicates=true;SetReplicateMovement(true);Health=MaxHealth;AmmoInMagazine=MagazineSize;ReserveAmmo=InitialReserveAmmo;}
void ACalamityCharacter::RequestFire(){if(!bEliminated){if(HasAuthority())FireAuthoritative();else ServerFire();}}
void ACalamityCharacter::ServerFire_Implementation(){FireAuthoritative();}
void ACalamityCharacter::RequestReload(){if(HasAuthority())ReloadAuthoritative();else ServerReload();}
void ACalamityCharacter::ServerReload_Implementation(){ReloadAuthoritative();}
void ACalamityCharacter::ReloadAuthoritative()
{
    if(!HasAuthority()||bEliminated||bIsReloading||AmmoInMagazine>=MagazineSize||ReserveAmmo<=0||!GetWorld())return;
    bIsReloading=true;FTimerHandle Timer;TWeakObjectPtr<ACalamityCharacter> Weak(this);
    GetWorldTimerManager().SetTimer(Timer,[Weak](){ACalamityCharacter* S=Weak.Get();if(!S||!S->HasAuthority())return;S->bIsReloading=false;if(S->bEliminated)return;const int32 N=FMath::Min(S->MagazineSize-S->AmmoInMagazine,S->ReserveAmmo);S->AmmoInMagazine+=N;S->ReserveAmmo-=N;},FMath::Max(0.1f,ReloadDurationSeconds),false);
}
#define CALAMITY_EMPTY_RPC(Request, Server) void ACalamityCharacter::Request(){if(HasAuthority())Server##_Implementation();else Server();}
CALAMITY_EMPTY_RPC(RequestBeginCrownChannel,ServerBeginCrownChannel)
CALAMITY_EMPTY_RPC(RequestCancelCrownChannel,ServerCancelCrownChannel)
CALAMITY_EMPTY_RPC(RequestPickupDroppedCrown,ServerPickupDroppedCrown)
CALAMITY_EMPTY_RPC(RequestAcceptAlliance,ServerAcceptAlliance)
CALAMITY_EMPTY_RPC(RequestBreakAlliance,ServerBreakAlliance)
#undef CALAMITY_EMPTY_RPC
void ACalamityCharacter::ServerBeginCrownChannel_Implementation(){if(!bEliminated&&GetController())if(auto* G=GetWorld()->GetAuthGameMode<ACalamityGameMode>())G->BeginCrownChannel(GetController());}
void ACalamityCharacter::ServerCancelCrownChannel_Implementation(){if(GetController())if(auto* G=GetWorld()->GetAuthGameMode<ACalamityGameMode>())G->CancelCrownChannel(GetController());}
void ACalamityCharacter::ServerPickupDroppedCrown_Implementation(){if(!bEliminated&&GetController())if(auto* G=GetWorld()->GetAuthGameMode<ACalamityGameMode>())G->TryPickupDroppedCrown(GetController());}
void ACalamityCharacter::RequestAllianceOffer(APlayerState* P){if(HasAuthority())ServerRequestAllianceOffer_Implementation(P);else ServerRequestAllianceOffer(P);}
void ACalamityCharacter::ServerRequestAllianceOffer_Implementation(APlayerState* P){if(!bEliminated&&GetController())if(auto* G=GetWorld()->GetAuthGameMode<ACalamityGameMode>())G->RequestAllianceOffer(GetController(),P);}
void ACalamityCharacter::ServerAcceptAlliance_Implementation(){if(!bEliminated&&GetController())if(auto* G=GetWorld()->GetAuthGameMode<ACalamityGameMode>())G->AcceptAllianceOffer(GetController());}
void ACalamityCharacter::ServerBreakAlliance_Implementation(){if(GetController())if(auto* G=GetWorld()->GetAuthGameMode<ACalamityGameMode>())G->BreakAlliance(GetController());}
void ACalamityCharacter::RequestTameCreature(ACalamityCreature* C){if(HasAuthority())ServerTameCreature_Implementation(C);else ServerTameCreature(C);}
void ACalamityCharacter::ServerTameCreature_Implementation(ACalamityCreature* C){if(!HasAuthority()||bEliminated||TamingItems<=0||!IsValid(C)||!GetController()||FVector::DistSquared(GetActorLocation(),C->GetActorLocation())>FMath::Square(250.0f))return;if(C->TryTame(GetController()))--TamingItems;}
int32 ACalamityCharacter::AddReserveAmmo(int32 Amount){if(!HasAuthority()||Amount<=0||bEliminated)return 0;const int32 Added=FMath::Min(Amount,FMath::Max(0,MaxReserveAmmo-ReserveAmmo));ReserveAmmo+=Added;return Added;}
void ACalamityCharacter::FireAuthoritative(){if(!HasAuthority()||bEliminated||bIsReloading||!GetWorld()||AmmoInMagazine<=0)return;const float Now=GetWorld()->GetTimeSeconds();if(Now-LastServerFireTime<FMath::Max(0.1f,FireIntervalSeconds))return;LastServerFireTime=Now;--AmmoInMagazine;const FVector Start=GetPawnViewLocation(),Dir=GetBaseAimRotation().Vector();FCollisionQueryParams Q(SCENE_QUERY_STAT(CalamityHitscan),true,this);Q.AddIgnoredActor(this);FHitResult Hit;if(GetWorld()->LineTraceSingleByChannel(Hit,Start,Start+Dir*WeaponRange,ECC_Visibility,Q)&&Hit.GetActor())UGameplayStatics::ApplyPointDamage(Hit.GetActor(),BaseWeaponDamage,Dir,Hit,GetController(),this,nullptr);}
float ACalamityCharacter::TakeDamage(float Amount,FDamageEvent const&,AController* Instigator,AActor*){if(!HasAuthority()||bEliminated||Amount<=0||!GetWorld())return 0; if(Instigator&&GetController())if(const auto* G=GetWorld()->GetAuthGameMode<ACalamityGameMode>();G&&!G->bAllowFriendlyFire&&G->AreAllied(Instigator->PlayerState,GetController()->PlayerState))return 0;const float Applied=FMath::Min(Health,Amount);Health=FMath::Max(0.0f,Health-Applied);if(Health<=0)Eliminate(Instigator);return Applied;}
void ACalamityCharacter::Eliminate(AController* K){if(!HasAuthority()||bEliminated)return;bEliminated=true;SetActorEnableCollision(false);if(auto* G=GetWorld()->GetAuthGameMode<ACalamityGameMode>())G->HandlePlayerEliminated(GetController(),K);}
void ACalamityCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& O)const{Super::GetLifetimeReplicatedProps(O);DOREPLIFETIME(ACalamityCharacter,MaxHealth);DOREPLIFETIME(ACalamityCharacter,Health);DOREPLIFETIME(ACalamityCharacter,bEliminated);DOREPLIFETIME(ACalamityCharacter,AmmoInMagazine);DOREPLIFETIME(ACalamityCharacter,ReserveAmmo);DOREPLIFETIME(ACalamityCharacter,TamingItems);}
