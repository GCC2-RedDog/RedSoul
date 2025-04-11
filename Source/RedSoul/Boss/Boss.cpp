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

ABoss::ABoss()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABoss::BeginPlay()
{
	Super::BeginPlay();

	HandAttackCollider = FindComponentByTag<UBoxComponent>("Hand");
	HandAttackCollider->OnComponentBeginOverlap.AddDynamic(this, &ABoss::OnHandAttackOverlapBegin);

	LightningExplosionAttackCollider = FindComponentByClass<USphereComponent>();
	LightningExplosionAttackCollider->OnComponentBeginOverlap.AddDynamic(
		this, &ABoss::OnLightningExplosionAttackOverlapBegin);

	DirectHitCollider = FindComponentByTag<UBoxComponent>("DirectHit");
	DirectHitCollider->OnComponentBeginOverlap.AddDynamic(this, &ABoss::OnMeshOverlapBegin);

	NS_LightningAura_L = FindComponentByTag<UNiagaraComponent>("LightningAura_L"); 
	NS_LightningAura_R = FindComponentByTag<UNiagaraComponent>("LightningAura_R"); 
	
	BossMesh = FindComponentByClass<USkeletalMeshComponent>();

	LightningExplosionMesh = Cast<UStaticMeshComponent>(
		FindComponentByTag(UStaticMeshComponent::StaticClass(), "Lightning"));

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
			//BossToTargetDir += FVector(0, 0, -980.0f); 
			// GetCharacterMovement()->AddImpulse(FVector(0, 0, -980.0f), true); 
			//LaunchCharacter(BossToTargetDir, false, false); 
			//GetCharacterMovement()->GravityScale *= 5.0f; 
			if (auto MC = GetMovementComponent())
			{
				FVector& Vel = MC->Velocity;
				Vel.Z = 0; 
				Vel = Vel.GetSafeNormal() * 2000.0f; 
				Vel.Z = -980.0f; 
			} 
			IsActiveAttack2 = false;
			IsAttack2Smash = true; 
		}
	}
	
	FVector Pos;
	FRotator Rot; 
	BossMesh->GetSocketWorldLocationAndRotation("LHand", Pos, Rot); 
	DrawDebugBox(GetWorld(), Pos, FVector(90, 90, 210), Rot.Quaternion(), FColor::Red); 
	BossMesh->GetSocketWorldLocationAndRotation("RHand", Pos, Rot);
	DrawDebugBox(GetWorld(), Pos, FVector(90, 90, 210), Rot.Quaternion(), FColor::Red); 
	BossMesh->GetSocketWorldLocationAndRotation("LHand", Pos, Rot); 
	DrawDebugBox(GetWorld(), Pos, FVector(75, 75, 180), Rot.Quaternion(), FColor::Red); 
	BossMesh->GetSocketWorldLocationAndRotation("RHand", Pos, Rot); 
	DrawDebugBox(GetWorld(), Pos, FVector(75, 75, 180), Rot.Quaternion(), FColor::Red); 
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
		CurHP -= AttackInfo.Damage;
		Cast<UBossUI>(BossInfoObject)->SetHPBar(CurHP / MaxHP);
		//GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, FString::Printf(TEXT("Boss Hit %f"), CurHP));

		if (AttackType == EAttackType::AT_None)
		{
			if (auto AI = BossMesh->GetAnimInstance())
			{
				AI->Montage_Play(Hit_Montage);
			}
		}

		if (!IsPhase2 && CurHP <= MaxHP * 5 / 10.0f)
		{ 
			IsPhase2 = true; 
			//GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, TEXT("Phase 2")); 

			if (auto AI = BossMesh->GetAnimInstance())
			{
				AI->Montage_Play(Phase2_Montage);
			} 

			Blackboard->SetValueAsBool("IsAnimPhase2", true);
			GetWorld()->GetTimerManager().SetTimer(Phase2TimerHandle, FTimerDelegate::CreateLambda([&]()
			{
				Blackboard->SetValueAsBool("IsPhase2", true); 
				Blackboard->SetValueAsBool("IsAnimPhase2", false); 

				NS_LightningAura_L->SetAutoActivate(true);  
				NS_LightningAura_R->SetAutoActivate(true);
				
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
	if (auto AI = BossMesh->GetAnimInstance())
	{
		AI->Montage_Play(Awake_Montage);
	}

	Player = OtherCharacter;

	GetWorld()->GetTimerManager().SetTimer(AwakeTimerHandle, FTimerDelegate::CreateLambda([&]()
	{
		IsAwake = true;
		Cast<AAIC_Boss>(GetController())->Awaken(Player);

		BossInfoObject->AddToViewport();
		Cast<UBossUI>(BossInfoObject)->SetHPBar(CurHP / MaxHP);


		GetWorld()->GetTimerManager().ClearTimer(AwakeTimerHandle);
	}), 4.5f, false);
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
		if (auto AI = BossMesh->GetAnimInstance())
		{
			AI->Montage_Play(Attack1_Montage);
		}
		break;
	case EAttackType::AT_Attack2:
		{
			GetController()->StopMovement();

			IsActiveAttack2 = true;

			FVector CalcVelocity(0);
			Attack2TargetLocation = Player->GetActorLocation() + FVector(0, 0, 250.0f); 
			UGameplayStatics::SuggestProjectileVelocity_CustomArc(GetWorld(), CalcVelocity, GetActorLocation(),
																  GetPlayerAround(-BossToPlayerDist / 1.5f), 0.0f,
																  0.8f); 
			LaunchCharacter(CalcVelocity, false, false); 
			break; 
		}
	case EAttackType::AT_Attack3:
		if (auto AI = BossMesh->GetAnimInstance())
		{
			AI->Montage_Play(Attack3_Montage);
		}
		break;
	case EAttackType::AT_Attack4:
		GetController()->StopMovement();

		if (auto AI = BossMesh->GetAnimInstance())
		{
			AI->Montage_Play(Attack4_Montage);
		}
		break;
	case EAttackType::AT_Attack5:
		IsActiveAttack5 = true;

		GetWorld()->GetTimerManager().SetTimer(Attack5TimerHandle, FTimerDelegate::CreateLambda([&]()
		{
			IsActiveAttack5 = false;

			GetWorld()->GetTimerManager().ClearTimer(Attack5TimerHandle);
		}), 0.1f, false);
		break;
	case EAttackType::AT_Attack6:
		if (auto AI = BossMesh->GetAnimInstance())
		{
			AI->Montage_Play(Attack6_Montage);
		}
		break;
	}
}

void ABoss::SetAttackState(EAttackHand Hand, bool IsHandAttack, bool State)
{
	switch (Hand)
	{
	case EAttackHand::AH_None:
		HandAttackCollider->AttachToComponent(BossMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, "Center");
		DirectHitCollider->AttachToComponent(BossMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, "Center");
		HandAttackCollider->SetRelativeScale3D(FVector(0.4f, 0.4f, 0.7f));
		DirectHitCollider->SetRelativeScale3D(FVector(0.4f, 0.4f, 0.7f));
		break;
	case EAttackHand::AH_Center:
		HandAttackCollider->AttachToComponent(BossMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, "Center");
		DirectHitCollider->AttachToComponent(BossMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, "Center");
		HandAttackCollider->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.7f));
		DirectHitCollider->SetRelativeScale3D(FVector(0.35f, 0.35f, 0.7f));
		break;
	case EAttackHand::AH_Left:
		HandAttackCollider->AttachToComponent(BossMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, "LHand");
		DirectHitCollider->AttachToComponent(BossMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, "LHand");
		HandAttackCollider->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.8f));
		DirectHitCollider->SetRelativeScale3D(FVector(0.35f, 0.35f, 0.7f));
		break;
	case EAttackHand::AH_Right:
		HandAttackCollider->AttachToComponent(BossMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, "RHand");
		DirectHitCollider->AttachToComponent(BossMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, "RHand");
		HandAttackCollider->SetRelativeScale3D(FVector(0.4f, 0.4f, 0.75f));
		DirectHitCollider->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.675f));
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

	//GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, State ? TEXT("AttackStart") : TEXT("AttackEnd")); 
}

void ABoss::SetIgnoreToPlayer(bool State)
{
	if (State) BossMesh->MoveIgnoreActors.Add(Player);
	else BossMesh->MoveIgnoreActors.Remove(Player); 
}

void ABoss::FocusToPlayer()
{
	IsFocusToPlayer = true;

	FocusToPlayerAngle = FMath::RadiansToDegrees(FMath::Acos(GetActorForwardVector().Dot(GetBossToPlayerDir())));

	float Dir = GetActorForwardVector().Cross(GetBossToPlayerDir()).Z;
	FocusToPlayerAngle *= Dir > 0 ? 1 : -1;

	if (abs(FocusToPlayerAngle) > 5.0f)
	{
		if (auto AI = BossMesh->GetAnimInstance())
		{
			AI->Montage_Play(Dir > 0 ? RTurn_Montage : LTurn_Montage);
		}
	}

	GetWorld()->GetTimerManager().SetTimer(FocusTimerHandle, FTimerDelegate::CreateLambda([&]()
	{
		IsFocusToPlayer = false;

		GetWorld()->GetTimerManager().ClearTimer(FocusTimerHandle);
	}), 0.4f, false);
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
		EAttackResult AR = Execute_Hit(Player, {10, false, false, false, 0, FVector(0), FVector(0)});
		if (AR == EAttackResult::AR_Death)
		{
			StopLogic(); 
		}
		
		GetWorld()->GetTimerManager().ClearTimer(ThrowTimerHandle);
	}), 0.25f, false);
}

void ABoss::CheckDirectHit()
{
	EAttackResult AR = EAttackResult::AR_None;

	switch (AttackType)
	{
	case EAttackType::AT_Attack1:
		{
			FHitResult Hit = GetHitResult(HandAttackCollider->GetComponentLocation(), Player->GetActorLocation());
			AR = Execute_Hit(Player, {IsPhase2 ? 15.0f : 10, true, IsOverlapMesh, false, 0, Hit.ImpactPoint, Hit.ImpactNormal});
			break;
		}
	case EAttackType::AT_Attack2:
		AR = Execute_Hit(Player, {10, false, IsOverlapMesh, false, 0, FVector(0), FVector(0)});
		break;
	case EAttackType::AT_Attack3:
		AR = Execute_Hit(Player, {IsPhase2 ? 15.0f : 10, true, IsOverlapMesh, false, 0, FVector(0), FVector(0)});
		LaunchPlayer(GetFistSwingDir(), 1500.0f);
		break;
	case EAttackType::AT_Attack4:
		AR = Execute_Hit(Player, {10, false, IsOverlapMesh, false, 0, FVector(0), FVector(0)});
		if (IsPhase2) LaunchPlayer(GetShoulderHitDir(), 750.0f);
		SetAttackState(EAttackHand::AH_None, true, false);
		GetCharacterMovement()->Velocity = FVector(0);
		break;
	}

	switch (AR)
	{
	case EAttackResult::AR_Parrying:
		if (auto AI = BossMesh->GetAnimInstance())
		{
			AI->Montage_Play(Parryed_Montage);
		} 

		Blackboard->SetValueAsBool("IsParryed", true);
		GetWorld()->GetTimerManager().SetTimer(ParryedTimerHandle, FTimerDelegate::CreateLambda([&]()
		{
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
	if (AttackType == EAttackType::AT_Attack6)
	{
		SetAttackState(EAttackHand::AH_None, false, false);
		Execute_Hit(Player, {10, false, false, true, 1.0f, FVector(0), FVector(0)});
	}
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
	//GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, TEXT("Boss Die")); 

	StopLogic(); 

	NS_LightningAura_L->SetAutoActivate(false);  
	NS_LightningAura_R->SetAutoActivate(false); 
	
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
