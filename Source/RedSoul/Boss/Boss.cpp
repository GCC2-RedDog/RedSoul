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

	LightningExplosionMesh = Cast<UStaticMeshComponent>(FindComponentByTag(UStaticMeshComponent::StaticClass(), "Lightning")); 

	BossInfoObject = CreateWidget<UBossUI>(GetWorld(), BossInfoWidget); 
	
	PreForward = GetActorForwardVector(); 
}

void ABoss::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime); 

	if (Player && Blackboard) { 
		float Distance = (GetActorLocation() - Player->GetActorLocation()).Length();
		Blackboard->SetValueAsFloat("BossToPlayerDistance", Distance);
	}

	FVector CurForward = GetActorForwardVector();
	if (CurForward != PreForward)
	{
		DeltaRotAngle = acosf(CurForward.Dot(PreForward)) * 180 / PI;  
		IsTurnLeft = !(PreForward.Cross(CurForward).Z > 0);   
		PreForward = CurForward;
	} 
	else
	{
		DeltaRotAngle = 0; 
	}
}

void ABoss::Hit_Implementation(FAttackInfo AttackInfo)
{ 
	if (IsAwake && !IsDie) { 
		CurHP -= AttackInfo.Damage;
		Cast<UBossUI>(BossInfoObject)->SetHPBar(CurHP / MaxHP);
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, FString::Printf(TEXT("Boss Hit %f"), CurHP));

		if (AttackType == EAttackType::AT_None)
		{
			if (auto AI = BossMesh->GetAnimInstance())
			{
				AI->Montage_Play(Hit_Montage); 
			}
		}

		if (!IsPhase2 && CurHP <= MaxHP * 5 / 10.0f) { 
			IsPhase2 = true; 
			Blackboard->SetValueAsBool("IsPhase2", true); 
			GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, TEXT("Phase 2")); 
		} 
		
		if (CurHP <= 0) { 
			IsDie = true; 
			Die(); 
		}
	} 
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

	if (auto AIC = Cast<AAIC_Boss>(GetController())) {
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
	case EAttackType::AT_Attack3: 
		if (auto AI = BossMesh->GetAnimInstance())
		{
			AI->Montage_Play(Attack3_Montage);
		} 
		break; 
	case EAttackType::AT_Attack4:
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
	if (State) SetBlockToPlayer(State); 
	
	switch (Hand)
	{
	case EAttackHand::AH_None:
		HandAttackCollider->AttachToComponent(BossMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, "Center");
		break;
	case EAttackHand::AH_Center:
		HandAttackCollider->AttachToComponent(BossMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, "Center");
		break;
	case EAttackHand::AH_Left:
		HandAttackCollider->AttachToComponent(BossMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, "LHand");
		break;
	case EAttackHand::AH_Right:
		HandAttackCollider->AttachToComponent(BossMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, "RHand");
		break; 
	}
	
	if (IsHandAttack) HandAttackCollider->SetGenerateOverlapEvents(State);
	else LightningExplosionAttackCollider->SetGenerateOverlapEvents(State); 
	SetActorLocation(GetActorLocation() + FVector(0.1f, 0, 0));

	if (!State && AttackType == EAttackType::AT_Attack5)
	{
		if (auto AI = BossMesh->GetAnimInstance())
		{
			IsAttack5Success = false;
		}
	} 

	if (!State) AttackType = EAttackType::AT_None;
	
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, State ? TEXT("AttackStart") : TEXT("AttackEnd")); 
} 

FVector ABoss::GetPlayerAround(float Distance) 
{
	return Player->GetActorLocation() - GetBossToPlayerDir() * Distance; 
} 

void ABoss::LaunchPlayer(FVector Dir, float Force)
{ 
	Player->LaunchCharacter(Dir * Force, false, false); 
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

	GetWorld()->GetTimerManager().SetTimer(ThrowTimerHandle, FTimerDelegate::CreateLambda([&]() {
		Execute_Hit(Player, { 10, false, 0 });

		GetWorld()->GetTimerManager().ClearTimer(ThrowTimerHandle); 
	}), 0.5f, false);
}

void ABoss::SetBlockToPlayer(bool State)
{
	if (State) BossMesh->MoveIgnoreActors.Add(Player);
	else BossMesh->MoveIgnoreActors.Remove(Player); 
}

void ABoss::OnHandAttackOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{ 
	switch (AttackType) {
	case EAttackType::AT_Attack1:
		Execute_Hit(Player, { IsPhase2 ? 15.0f : 10.0f, false, 0 });
		break;
	case EAttackType::AT_Attack2:
		Execute_Hit(Player, { 10, false, 0 }); 
		break;
	case EAttackType::AT_Attack3:
		Execute_Hit(Player, { IsPhase2 ? 15.0f : 10.0f, true, 2 });
		LaunchPlayer(GetFistSwingDir(), 2000.0f);
		break;
	case EAttackType::AT_Attack4:
		Execute_Hit(Player, { 10, false,  0 }); 
		LaunchPlayer(GetShoulderHitDir(), 750.0f); 
		SetAttackState(EAttackHand::AH_None, true, false); 
		break;
	case EAttackType::AT_Attack5:
		if (auto AIC = Cast<AAIC_Boss>(GetController())) { 
			AIC->ClearFocus(2);
		} 
		
		IsAttack5Success = true; 
		break;
	}
}

void ABoss::OnLightningExplosionAttackOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{ 
	if (AttackType == EAttackType::AT_Attack6)
	{
		SetAttackState(EAttackHand::AH_None, false, false); 
		Execute_Hit(Player, { 10, false, 0 }); 
	}
} 

void ABoss::Die()
{ 
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, TEXT("Boss Die")); 

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
	return GetBossToPlayerDir() + FVector(0, 0, 0.1f); 
}

FVector ABoss::GetShoulderDir()
{
	return (GetBossToPlayerDir() + FVector(0.0f, 0.0f, 0.05f)) * 3000.0f; 
}

FVector ABoss::GetThrowPlayerDir()
{
	return GetBossToPlayerDir() - FVector(0.0f, 0.0f, 0.65f); 
}

FVector ABoss::GetShoulderHitDir()
{
	return GetBossToPlayerDir() + FVector(0.0f, 0.0f, 0.2f); 
}
