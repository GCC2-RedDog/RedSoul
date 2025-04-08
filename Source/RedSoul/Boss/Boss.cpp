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
	LightningExplosionAttackCollider->OnComponentBeginOverlap.AddDynamic(
		this, &ABoss::OnLightningExplosionAttackOverlapBegin);

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

	if (IsActiveAttack2 && BossToPlayerDist < 500.0f)
	{
		if (auto MC = GetMovementComponent())
		{
			MC->Velocity /= 5.0f;
			MC->Velocity.Z -= 980.0f; 
		}
	}
	
	//FVector Pos;
	//FRotator Rot; 
	//BossMesh->GetSocketWorldLocationAndRotation("LHand", Pos, Rot); 
	//DrawDebugBox(GetWorld(), Pos, FVector(60, 90, 210), Rot.Quaternion(), FColor::Red); 
	//BossMesh->GetSocketWorldLocationAndRotation("RHand", Pos, Rot);
	//DrawDebugBox(GetWorld(), Pos, FVector(60, 90, 210), Rot.Quaternion(), FColor::Red); 
}

void ABoss::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearTimer(AwakeTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(Attack2TimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(Attack4TimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(Attack5TimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(Attack6TimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(ThrowTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(FocusTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(HitTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(StunTimerHandle);
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
			Blackboard->SetValueAsBool("IsPhase2", true);
			//GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, TEXT("Phase 2")); 
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

		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), NS_StoneParts, AttackInfo.HitPos,
		                                               AttackInfo.HitNormal.Rotation());
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

			GetWorld()->GetTimerManager().SetTimer(Attack2TimerHandle, FTimerDelegate::CreateLambda([&]()
			{
				FVector CalcVelocity(0);
				UGameplayStatics::SuggestProjectileVelocity_CustomArc(GetWorld(), CalcVelocity, GetActorLocation(),
				                                                      GetPlayerAround(-BossToPlayerDist / 2), 0.0f,
				                                                      0.5f);
				LaunchCharacter(CalcVelocity, false, false);

				GetWorld()->GetTimerManager().ClearTimer(Attack2TimerHandle);
			}), 0.1f, false);

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
		}), 0.2f, false);
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
	if (State) SetIgnoreToPlayer(State);

	switch (Hand)
	{
	case EAttackHand::AH_None:
		HandAttackCollider->AttachToComponent(BossMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		                                      "Center");
		HandAttackCollider->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));
		break;
	case EAttackHand::AH_Center:
		HandAttackCollider->AttachToComponent(BossMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		                                      "Center");
		HandAttackCollider->SetRelativeScale3D(FVector(0.7f, 0.7f, 0.7f));
		break;
	case EAttackHand::AH_Left:
		HandAttackCollider->AttachToComponent(BossMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		                                      "LHand");
		HandAttackCollider->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.8f));
		break;
	case EAttackHand::AH_Right:
		HandAttackCollider->AttachToComponent(BossMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		                                      "RHand");
		HandAttackCollider->SetRelativeScale3D(FVector(0.4f, 0.4f, 0.75f));
		break;
	}

	if (IsHandAttack) HandAttackCollider->SetGenerateOverlapEvents(State);
	else LightningExplosionAttackCollider->SetGenerateOverlapEvents(State);
	SetActorLocation(GetActorLocation() + FVector(0.1f, 0, 0));

	if (!State && AttackType == EAttackType::AT_Attack5)
	{
		IsAttack5Success = false;
	}

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

	if (auto AI = BossMesh->GetAnimInstance())
	{
		AI->Montage_Play(Dir > 0 ? RTurn_Montage : LTurn_Montage);
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

	Player->GetComponentByClass<UCharacterMovementComponent>()->GravityScale = 0.0f;
	Player->GetComponentByClass<UCharacterMovementComponent>()->MaxWalkSpeed = 0.0f;
}

void ABoss::PlayerThrow()
{
	Player->GetComponentByClass<UCharacterMovementComponent>()->GravityScale = 1.75f;
	Player->GetComponentByClass<UCharacterMovementComponent>()->MaxWalkSpeed = 500.0f;

	Player->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	LaunchPlayer(GetThrowPlayerDir(), 1500.0f);

	GetWorld()->GetTimerManager().SetTimer(ThrowTimerHandle, FTimerDelegate::CreateLambda([&]()
	{
		Execute_Hit(Player, {10, false, false, false, 0, FVector(0), FVector(0)});

		GetWorld()->GetTimerManager().ClearTimer(ThrowTimerHandle);
	}), 0.25f, false);
}

FVector ABoss::GetPlayerAround(float Distance)
{
	return Player->GetActorLocation() - GetBossToPlayerDir() * Distance;
}

void ABoss::OnHandAttackOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                     UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                     const FHitResult& SweepResult)
{
	EAttackResult AR = EAttackResult::AR_None;

	switch (AttackType)
	{
	case EAttackType::AT_Attack1:
		AR = Execute_Hit(Player, {IsPhase2 ? 15.0f : 10, true, false, false, 0, FVector(0), FVector(0)});
		break;
	case EAttackType::AT_Attack2:
		AR = Execute_Hit(Player, {10, false, false, false, 0, FVector(0), FVector(0)});
		break;
	case EAttackType::AT_Attack3:
		AR = Execute_Hit(Player, {IsPhase2 ? 15.0f : 10, true, false, false, 0, FVector(0), FVector(0)});
		LaunchPlayer(GetFistSwingDir(), 1500.0f);
		break;
	case EAttackType::AT_Attack4:
		Execute_Hit(Player, {10, false, false, false, 0, FVector(0), FVector(0)});
		if (IsPhase2) LaunchPlayer(GetShoulderHitDir(), 750.0f);
		SetAttackState(EAttackHand::AH_None, true, false);
		GetCharacterMovement()->Velocity = FVector(0);
		break;
	case EAttackType::AT_Attack5:
		if (auto AIC = Cast<AAIC_Boss>(GetController()))
		{
			AIC->ClearFocus(2);
		}

		IsAttack5Success = true;
		break;
	}

	if (AR == EAttackResult::AR_Parrying)
	{
		if (auto AI = BossMesh->GetAnimInstance())
		{
			AI->Montage_Play(Stun_Montage);
		}

		Blackboard->SetValueAsBool("IsStun", true);
		GetWorld()->GetTimerManager().SetTimer(StunTimerHandle, FTimerDelegate::CreateLambda([&]()
		{
			Blackboard->SetValueAsBool("IsStun", false);

			GetWorld()->GetTimerManager().ClearTimer(StunTimerHandle);
		}), 2.0f, false);
	}
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

void ABoss::LaunchPlayer(FVector Dir, float Force)
{
	Player->LaunchCharacter(Dir * Force, false, false);
}

void ABoss::Die()
{
	//GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, TEXT("Boss Die")); 

	if (auto AIC = Cast<AAIC_Boss>(GetController()))
	{
		AIC->ClearFocus(2);
		AIC->GetBrainComponent()->StopLogic(TEXT("Die"));
	}

	BossInfoObject->RemoveFromParent();
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
	return GetBossToPlayerDir() - FVector(0.0f, 0.0f, 0.35f);
}

FVector ABoss::GetShoulderHitDir()
{
	return GetBossToPlayerDir() + FVector(0.0f, 0.0f, 0.35f);
}
