// Fill out your copyright notice in the Description page of Project Settings.


#include "Boss.h" 
#include "BossUI.h" 
#include "AIC_Boss.h" 
#include "BehaviorTree/BlackboardComponent.h" 
#include "Components/BoxComponent.h" 
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h" 
#include "Kismet/GameplayStatics.h"

ABoss::ABoss()
{
 	PrimaryActorTick.bCanEverTick = true;

}

void ABoss::BeginPlay() 
{
	Super::BeginPlay(); 

	HandAttackCollider = FindComponentByClass<UBoxComponent>(); 
	HandAttackCollider->OnComponentBeginOverlap.AddDynamic(this, &ABoss::OnHandAttackOverlapBegin);

	LightningExplosionAttackCollider = FindComponentByClass<USphereComponent>();
	LightningExplosionAttackCollider->OnComponentBeginOverlap.AddDynamic(this, &ABoss::OnLightningExplosionAttackOverlapBegin); 
	
	BossMesh = FindComponentByClass<USkeletalMeshComponent>();

	TempMesh = Cast<UStaticMeshComponent>(FindComponentByTag(UStaticMeshComponent::StaticClass(), "TempMesh")); 
	LightningExplosionMesh = Cast<UStaticMeshComponent>(FindComponentByTag(UStaticMeshComponent::StaticClass(), "Lightning")); 

	BossInfoObject = CreateWidget<UBossUI>(GetWorld(), BossInfoWidget); 
	BossInfoObject->AddToViewport(); 
	Cast<UBossUI>(BossInfoObject)->SetHPBar(CurHP / MaxHP); 
}

void ABoss::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime); 

	if (Player && Blackboard) { 
		float Distance = (GetActorLocation() - Player->GetActorLocation()).Length();
		Blackboard->SetValueAsFloat("BossToPlayerDistance", Distance);
	} 

	if (!IsPhase2 && CurHP <= MaxHP * 5 / 10.0f) { 
		IsPhase2 = true; 
		Blackboard->SetValueAsBool("IsPhase2", true); 
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, TEXT("Phase 2")); 
	} 
}

void ABoss::Hit_Implementation(FAttackInfo AttackInfo)
{ 
	if (IsAwake) {
		CurHP -= AttackInfo.Damage;
		Cast<UBossUI>(BossInfoObject)->SetHPBar(CurHP / MaxHP);
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, FString::Printf(TEXT("Boss Hit %f"), CurHP));

		if (CurHP <= 0) {
			Die();
		}
	} 
}

void ABoss::Interaction_Implementation(ACharacter* OtherCharacter)
{ 
	IsAwake = true; 
	Cast<AAIC_Boss>(GetController())->Awaken(OtherCharacter); 
}

void ABoss::Attack(EAttackType Value)
{
	AttackType = Value;

	if (auto AIC = Cast<AAIC_Boss>(GetController())) {
		AIC->ClearFocus(2);
	}
	 
	switch (Value)
	{
	case EAttackType::Attack1: 
		if (auto AI = BossMesh->GetAnimInstance())
		{
			AI->Montage_Play(Attack1_Montage);
		} 
		break; 
	case EAttackType::Attack2:
		{
			GetController()->StopMovement();

			FVector CalcVelocity(0); 
			UGameplayStatics::SuggestProjectileVelocity_CustomArc(GetWorld(), CalcVelocity, GetActorLocation(), GetPlayerAround(100.0f), 0.0f, 0.5f);
			LaunchCharacter(CalcVelocity, false, false);

			IsActiveAttack2 = true;

			GetWorld()->GetTimerManager().SetTimer(Attack2TimerHandle, FTimerDelegate::CreateLambda([&]()
			{
				IsActiveAttack2 = false; 
				
				GetWorld()->GetTimerManager().ClearTimer(Attack2TimerHandle); 
			}), 0.2f, false); 
			
			break;
		}
	case EAttackType::Attack3: 
		if (auto AI = BossMesh->GetAnimInstance())
		{
			AI->Montage_Play(Attack3_Montage);
		} 
		break; 
	case EAttackType::Attack4:
		if (auto AI = BossMesh->GetAnimInstance())
		{
			AI->Montage_Play(Attack4_Montage);
		} 
		break; 
	case EAttackType::Attack5:
		SetAttackState(true, true);
		GetWorld()->GetTimerManager().SetTimer(Attack5TimerHandle, FTimerDelegate::CreateLambda([&]()
		{
			SetAttackState(true, false); 
			
			GetWorld()->GetTimerManager().ClearTimer(Attack5TimerHandle); 
		}), 0.5f, false); 
		break; 
	case EAttackType::Attack6:
		AttackType = Value;
		LightningExplosionMesh->SetVisibility(true);
		SetAttackState(false, true); 

		GetWorld()->GetTimerManager().SetTimer(Attack6TimerHandle, FTimerDelegate::CreateLambda([&]()
		{
			LightningExplosionMesh->SetVisibility(false);
			SetAttackState(false, false); 
			
			GetWorld()->GetTimerManager().ClearTimer(Attack6TimerHandle); 
		}), 0.5f, false); 
		break; 
	}
}

void ABoss::SetAttackState(bool IsHandAttack, bool State)
{
	if (IsHandAttack) HandAttackCollider->SetGenerateOverlapEvents(State);
	else LightningExplosionAttackCollider->SetGenerateOverlapEvents(State); 
	SetActorLocation(GetActorLocation() + FVector(0.1f, 0, 0)); 
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, State ? TEXT("AttackStart") : TEXT("AttackEnd")); 
	//TempMesh->SetMaterial(0, State ? M_Attacking : M_Default);
} 

FVector ABoss::GetPlayerAround(float Distance) 
{
	return Player->GetActorLocation() - GetBossToPlayerDir() * Distance; 
} 

void ABoss::LaunchPlayer(FVector Dir, float Force)
{ 
	Player->LaunchCharacter(Dir * Force, false, false); 
}

void ABoss::OnHandAttackOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{ 
	switch (AttackType) {
	case EAttackType::Attack1:
		Execute_Hit(Player, { 10, false, 0 });
		break;
	case EAttackType::Attack2:
		Execute_Hit(Player, { 10, false, 0 }); 
		break;
	case EAttackType::Attack3:
		Execute_Hit(Player, { 10, true, 2 });
		LaunchPlayer(GetFistSwingDir(), 2000.0f);
		break;
	case EAttackType::Attack4:
		Execute_Hit(Player, { 10, false,  0 }); 
		break;
	case EAttackType::Attack5:
		if (auto AIC = Cast<AAIC_Boss>(GetController())) {
			AIC->ClearFocus(2);
		}
		
		Player->AttachToComponent(BossMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, "CatchedPosition"); 

		Player->GetComponentByClass<UCharacterMovementComponent>()->GravityScale = 0.0f;
		Player->GetComponentByClass<UCharacterMovementComponent>()->MaxWalkSpeed = 0.0f;

		GetWorld()->GetTimerManager().SetTimer(CatchTimerHandle, FTimerDelegate::CreateLambda([&]() {
			Player->GetComponentByClass<UCharacterMovementComponent>()->GravityScale = 1.75f; 
			Player->GetComponentByClass<UCharacterMovementComponent>()->MaxWalkSpeed = 500.0f;
			
			Player->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

			LaunchPlayer(GetThrowPlayerDir(), 1500.0f);

			GetWorld()->GetTimerManager().SetTimer(ThrowTimerHandle, FTimerDelegate::CreateLambda([&]() {
				Execute_Hit(Player, { 10, false, 0 });

				GetWorld()->GetTimerManager().ClearTimer(ThrowTimerHandle); 
				}), 0.5f, false);

			GetWorld()->GetTimerManager().ClearTimer(CatchTimerHandle); 
			}), 0.75f, false);
		break;
	}
}

void ABoss::OnLightningExplosionAttackOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{ 
	if (AttackType == EAttackType::Attack6)
	{
		Execute_Hit(Player, { 10, false, 0 }); 
	}
} 

void ABoss::Die()
{ 
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, TEXT("Boss Die")); 

	Destroy(); 
} 

FVector ABoss::GetBossToPlayerDir() 
{
	FVector Dir = Player->GetActorLocation() - GetActorLocation(); 
	Dir.Z = 0.0f; 
	Dir.Normalize();

	return Dir;
}

FVector ABoss::GetFistSwingDir()
{ 
	return HandAttackCollider->GetForwardVector() + FVector(0, 0, 0.1f); 
}

FVector ABoss::GetShoulderDir()
{
	return (GetBossToPlayerDir() + FVector(0.0f, 0.0f, 0.1f)) * 1500.0f; 
}

FVector ABoss::GetThrowPlayerDir()
{
	return GetBossToPlayerDir() - FVector(0.0f, 0.0f, 0.65f); 
}
