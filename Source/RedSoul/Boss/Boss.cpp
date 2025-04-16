// Fill out your copyright notice in the Description page of Project Settings.


#include "Boss.h"
#include "BossUI.h"
#include "AIC_Boss.h"
#include "BrainComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h" 
#include "Components/CapsuleComponent.h" 

ABoss::ABoss()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABoss::BeginPlay()
{
	Super::BeginPlay();

	CurHP = MaxHP; 
	
	HandAttackCollider = FindComponentByTag<UBoxComponent>("Hand");
	HandAttackCollider->OnComponentBeginOverlap.AddDynamic(this, &ABoss::OnHandAttackOverlapBegin);

	LightningExplosionAttackCollider = FindComponentByClass<USphereComponent>();
	LightningExplosionAttackCollider->OnComponentBeginOverlap.AddDynamic(
		this, &ABoss::OnLightningExplosionAttackOverlapBegin);

	DirectHitCollider = FindComponentByTag<UBoxComponent>("DirectHit");
	DirectHitCollider->OnComponentBeginOverlap.AddDynamic(this, &ABoss::OnMeshOverlapBegin);

	NS_LightningAura_L = FindComponentByTag<UNiagaraComponent>("LightningAura_L"); 
	NS_LightningAura_R = FindComponentByTag<UNiagaraComponent>("LightningAura_R"); 
	NS_LightningAura_L->Deactivate(); 
	NS_LightningAura_R->Deactivate(); 

	NS_LightningExplosion = FindComponentByTag<UNiagaraComponent>("LightningExplosion"); 
	NS_LightningExplosion->Deactivate(); 

	BossMesh = FindComponentByClass<USkeletalMeshComponent>(); 

	BossInfoObject = CreateWidget<UBossUI>(GetWorld(), BossInfoWidget);
}

void ABoss::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Player && Blackboard)
	{
		BossToPlayerDist = (GetActorLocation() - Player->GetActorLocation()).Length();
		Blackboard->SetValueAsFloat("BossToPlayerDistance", BossToPlayerDist);
	}

	if (IsFocusToPlayer)
	{
		AddActorLocalRotation(FRotator(0.0f, FocusToPlayerAngle * DeltaTime * 2.5f, 0.0f));
	} 

	if (IsActiveAttack2)
	{ 
		FVector BossToTargetDir = Attack2TargetLocation - GetActorLocation(); 
		if (BossToTargetDir.Length() < 750.0f)
		{ 
			if (auto MC = GetMovementComponent())
			{
				FVector& Vel = MC->Velocity;
				Vel.Z = 0; 
				Vel = Vel.GetSafeNormal() * 2000.0f; 
				Vel.Z = -980.0f; 
			} 
			IsActiveAttack2 = false; 
		}
	}
}

void ABoss::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearTimer(AwakeTimerHandle); 
	GetWorld()->GetTimerManager().ClearTimer(Attack4TimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(Attack5TimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(Attack6TimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(ThrowTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(FocusTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(HitTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(ParryedTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(AttackTimerHandle);
}

EAttackResult ABoss::Hit_Implementation(FAttackInfo AttackInfo)
{
	if (IsAwake && !IsDie && !IsHit)
	{
		CurHP -= AttackInfo.Damage * (IsParryed ? 2.0f : 1.0f);
		Cast<UBossUI>(BossInfoObject)->SetHPBar(CurHP / MaxHP);
		
		if (AttackType == EAttackType::AT_None && !IsParryed) 
		{
			PlayMontage(Hit_Montage); 
		}

		if (!IsPhase2 && CurHP <= MaxHP * 5 / 10.0f)
		{ 
			IsPhase2 = true; 
			
			PlayMontage(Phase2_Montage); 

			Blackboard->SetValueAsBool("IsAnimPhase2", true);
			GetWorld()->GetTimerManager().SetTimer(Phase2TimerHandle, FTimerDelegate::CreateLambda([&]()
			{
				Blackboard->SetValueAsBool("IsPhase2", true); 
				Blackboard->SetValueAsBool("IsAnimPhase2", false); 

				NS_LightningAura_L->Activate(true); 
	 			NS_LightningAura_R->Activate(true); 

				GetCapsuleComponent()->SetCapsuleRadius(175.0f); 

				GetWorld()->GetTimerManager().ClearTimer(Phase2TimerHandle);
			}), 3.0f, false);
		}

		if (CurHP <= 0)
		{
			IsDie = true;
			Die();

			return EAttackResult::AR_Death; 
		}

		IsHit = true;
		GetWorld()->GetTimerManager().SetTimer(HitTimerHandle, FTimerDelegate::CreateLambda([&]()
		{
			IsHit = false; 

			GetWorld()->GetTimerManager().ClearTimer(HitTimerHandle);
		}), 0.2f, false);

		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), NS_StoneParts, AttackInfo.HitPos, AttackInfo.HitNormal.Rotation());
	}
	return EAttackResult::AR_None;
}

void ABoss::Interaction_Implementation(ACharacter* OtherCharacter)
{
	PlayMontage(Awake_Montage); 

	Player = OtherCharacter;

	GetWorld()->GetTimerManager().SetTimer(AwakeTimerHandle, FTimerDelegate::CreateLambda([&]()
	{
		IsAwake = true;
		Cast<AAIC_Boss>(GetController())->Awaken(Player);

		BossInfoObject->AddToViewport();
		Cast<UBossUI>(BossInfoObject)->SetHPBar(CurHP / MaxHP);

		GetWorld()->GetTimerManager().ClearTimer(AwakeTimerHandle);
	}), 3.75f, false);
} 

void ABoss::Attack(EAttackType Value)
{
	AttackType = Value;

	if (auto AIC = Cast<AAIC_Boss>(GetController()))
	{
		AIC->ClearFocus(2);
	} 

	switch (Value)
	{
	case EAttackType::AT_Attack1:
		PlayMontage(Attack1_Montage); 
		break;
	case EAttackType::AT_Attack2:
		{
			GetController()->StopMovement();

			IsActiveAttack2 = true;

			FVector CalcVelocity(0);
			Attack2TargetLocation = Player->GetActorLocation() + FVector(0, 0, 250.0f); 
			UGameplayStatics::SuggestProjectileVelocity_CustomArc(GetWorld(), CalcVelocity, GetActorLocation(),
																  GetPlayerAround(-BossToPlayerDist / (BossToPlayerDist >= 1900.0f ? 1.5f : 3.0f)), 0.0f,
																  BossToPlayerDist >= 1900.0f ? 0.75f : 0.6f); 
			LaunchCharacter(CalcVelocity, false, false); 
			break; 
		}
	case EAttackType::AT_Attack3:
		PlayMontage(Attack3_Montage); 
		break;
	case EAttackType::AT_Attack4:
		GetController()->StopMovement();
		PlayMontage(Attack4_Montage); 
		break;
	case EAttackType::AT_Attack5:
		IsActiveAttack5 = true; 
		break;
	case EAttackType::AT_Attack6:
		PlayMontage(Attack6_Montage); 
		break;
	}
}

void ABoss::SetAttackState(EAttackHand Hand, bool IsHandAttack, bool State)
{
	switch (Hand)
	{
	case EAttackHand::AH_None:
		SetHandCollider("Center", FVector(0.5f, 0.5f, 0.7f)); 
		break;
	case EAttackHand::AH_Center:
		SetHandCollider("Center", FVector(0.6f, 0.6f, 0.7f)); 
		break;
	case EAttackHand::AH_Left:
		SetHandCollider("LHand", FVector(0.5f, 0.5f, 0.7f)); 
		break;
	case EAttackHand::AH_Right:
		SetHandCollider("RHand", FVector(0.5f, 0.5f, 0.7f)); 
		break;
	}

	if (IsHandAttack)
	{
		HandAttackCollider->SetGenerateOverlapEvents(State); 
		DirectHitCollider->SetGenerateOverlapEvents(State); 
	}
	else LightningExplosionAttackCollider->SetGenerateOverlapEvents(State);
	SetActorLocation(GetActorLocation() + FVector(0.1f, 0, 0)); 

	if (!State) AttackType = EAttackType::AT_None;

}

void ABoss::SetIgnoreToPlayer(bool State)
{
	if (State) BossMesh->MoveIgnoreActors.Add(Player);
	else BossMesh->MoveIgnoreActors.Remove(Player); 
}

void ABoss::FocusToPlayer()
{
	if (!IsFocusToPlayer)
	{
		IsFocusToPlayer = true;

		FocusToPlayerAngle = FMath::RadiansToDegrees(FMath::Acos(GetActorForwardVector().Dot(GetBossToPlayerDir())));

		float Dir = GetActorForwardVector().Cross(GetBossToPlayerDir()).Z;
		FocusToPlayerAngle *= Dir > 0 ? 1 : -1;

		if (abs(FocusToPlayerAngle) > 5.0f)
		{
			PlayMontage(Dir > 0 ? RTurn_Montage : LTurn_Montage); 
		}

		GetWorld()->GetTimerManager().SetTimer(FocusTimerHandle, FTimerDelegate::CreateLambda([&]()
		{
			IsFocusToPlayer = false;

			GetWorld()->GetTimerManager().ClearTimer(FocusTimerHandle);
		}), 0.5f, false);
	}
}

void ABoss::PlayerCatch()
{
	Player->AttachToComponent(BossMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, "CatchedPosition");

	if (auto CM = Player->GetCharacterMovement())
	{
		CM->GravityScale = 0.0f;
		CM->MaxWalkSpeed = 0.0f;
	}
}

void ABoss::PlayerThrow()
{
	if (auto CM = Player->GetCharacterMovement())
	{
		CM->GravityScale = 1.75f;
		CM->MaxWalkSpeed = 500.0f;
	}
	
	Player->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	LaunchPlayer(GetThrowPlayerDir(), 1500.0f);

	GetWorld()->GetTimerManager().SetTimer(ThrowTimerHandle, FTimerDelegate::CreateLambda([&]()
	{
		EAttackResult AR = Execute_Hit(Player, { 25.0f, false, true, false, 0, FVector(0), FVector(0) });
		if (AR == EAttackResult::AR_Death)
		{
			StopLogic(); 
		}
		
		GetWorld()->GetTimerManager().ClearTimer(ThrowTimerHandle);
	}), 0.2f, false);
}

void ABoss::CheckDirectHit()
{
	EAttackResult AR = EAttackResult::AR_None;
	FHitResult Hit = GetHitResult(HandAttackCollider->GetComponentLocation(), Player->GetActorLocation());

	switch (AttackType)
	{
	case EAttackType::AT_Attack1:
		AR = Execute_Hit(Player, { IsPhase2 ? IsOverlapMesh ? 64.0f : 33.0f : IsOverlapMesh ? 46.0f : 28.0f, true, IsOverlapMesh, false, 0, Hit.ImpactPoint, Hit.ImpactNormal });
		break;
	case EAttackType::AT_Attack2:
		AR = Execute_Hit(Player, { IsOverlapMesh ? 68.0f : 43.0f, false, IsOverlapMesh, false, 0, Hit.ImpactPoint, Hit.ImpactNormal });
		break;
	case EAttackType::AT_Attack3:
		AR = Execute_Hit(Player, { IsPhase2 ? IsOverlapMesh ? 48.0f : 29.0f : IsOverlapMesh ? 33.0f : 24.0f, true, IsOverlapMesh, false, 0, Hit.ImpactPoint, Hit.ImpactNormal });
		if (AR == EAttackResult::AR_None && IsOverlapMesh) LaunchPlayer(GetFistSwingDir(), 1500.0f);
		break;
	case EAttackType::AT_Attack4:
		AR = Execute_Hit(Player, { IsPhase2 ? IsOverlapMesh ? 43.0f : 26.0f : IsOverlapMesh ? 37.0f : 21.0f, false, IsOverlapMesh, false, 0, Hit.ImpactPoint, Hit.ImpactNormal });
		if (IsPhase2) LaunchPlayer(GetShoulderHitDir(), 750.0f);
		SetAttackState(EAttackHand::AH_None, true, false);
		GetCharacterMovement()->Velocity = FVector(0);
		break;
	}

	switch (AR)
	{
	case EAttackResult::AR_Parrying:
		PlayMontage(Parryed_Montage); 

		IsParryed = true; 
		Blackboard->SetValueAsBool("IsParryed", true);
		GetWorld()->GetTimerManager().SetTimer(ParryedTimerHandle, FTimerDelegate::CreateLambda([&]()
		{ 
				IsParryed = false; 
			Blackboard->SetValueAsBool("IsParryed", false);

			GetWorld()->GetTimerManager().ClearTimer(ParryedTimerHandle);
		}), 4.0f, false);
		break;
	case EAttackResult::AR_Death:
		StopLogic(); 
		break; 
	}

	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, IsOverlapMesh ? TEXT("DirectHit") : TEXT("Normal Hit")); 

	IsOverlapHand = false;
	IsOverlapMesh = false;
}

FVector ABoss::GetPlayerAround(float Distance)
{
	return Player->GetActorLocation() - GetBossToPlayerDir() * Distance;
}

FHitResult ABoss::GetHitResult(FVector AttackColliderLocation, FVector HitActorLocation)
{
	FHitResult Hit;
	FCollisionObjectQueryParams COQP;
	FCollisionQueryParams CQP; 

	COQP.AddObjectTypesToQuery(ECollisionChannel::ECC_GameTraceChannel1);
	CQP.AddIgnoredActor(this); 
	
	GetWorld()->LineTraceSingleByObjectType(Hit, AttackColliderLocation, HitActorLocation, COQP, CQP);

	return Hit; 
}

void ABoss::OnHandAttackOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                     UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                     const FHitResult& SweepResult)
{
	switch (AttackType)
	{
	case EAttackType::AT_Attack1:
	case EAttackType::AT_Attack2:
	case EAttackType::AT_Attack3:
	case EAttackType::AT_Attack4:
		IsOverlapHand = true;

		GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, FTimerDelegate::CreateLambda([&]()
		{
			CheckDirectHit(); 
			
			GetWorld()->GetTimerManager().ClearTimer(AttackTimerHandle); 
		}), 0.01f, false); 
		break; 
	case EAttackType::AT_Attack5:
		if (auto AIC = Cast<AAIC_Boss>(GetController()))
		{
			AIC->ClearFocus(2);
		}

		IsAttack5Success = true;
		break;
	}

	SetIgnoreToPlayer(true);
}

void ABoss::OnLightningExplosionAttackOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	SetAttackState(EAttackHand::AH_None, false, false);
	Execute_Hit(Player, { 39.0f, false, false, true, 1.0f, FVector(0), FVector(0) });
} 

void ABoss::OnMeshOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IsOverlapHand)
	{
		IsOverlapMesh = true; 
	}
}

void ABoss::LaunchPlayer(FVector Dir, float Force)
{
	Player->LaunchCharacter(Dir * Force, false, false);
}

void ABoss::Die()
{ 
	StopLogic(); 

	NS_LightningAura_L->Deactivate(); 
	NS_LightningAura_R->Deactivate(); 

	BossInfoObject->RemoveFromParent();
}

void ABoss::StopLogic()
{ 
	if (auto AIC = Cast<AAIC_Boss>(GetController()))
	{
		AIC->ClearFocus(2);
		AIC->GetBrainComponent()->StopLogic(TEXT("Die"));
	}
}

void ABoss::SetHandCollider(FString Socket, FVector Scale)
{ 
	if (IsPhase2) Scale += FVector(0.165f); 

	HandAttackCollider->AttachToComponent(BossMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, *Socket);
	DirectHitCollider->AttachToComponent(BossMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, *Socket);
	HandAttackCollider->SetRelativeScale3D(Scale);
	DirectHitCollider->SetRelativeScale3D(Scale - FVector(0.1f)); 
}

void ABoss::PlayMontage(UAnimMontage* Montage)
{ 
	if (auto AI = BossMesh->GetAnimInstance())
	{
		AI->Montage_Play(Montage);
	}
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
	return GetBossToPlayerDir() + FVector(0, 0, 0.05f);
}

FVector ABoss::GetShoulderDir()
{
	return (GetBossToPlayerDir() + FVector(0.0f, 0.0f, 0.05f)) * 3000.0f;
}

FVector ABoss::GetThrowPlayerDir()
{
	return GetBossToPlayerDir() - FVector(0.0f, 0.0f, 0.25f);
}

FVector ABoss::GetShoulderHitDir()
{
	return GetBossToPlayerDir() + FVector(0.0f, 0.0f, 0.35f);
}
