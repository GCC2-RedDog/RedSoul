// Fill out your copyright notice in the Description page of Project Settings.


#include "Boss.h" 
#include "BossUI.h" 
#include "AIC_Boss.h" 
#include "BehaviorTree/BlackboardComponent.h" 
#include "Components/BoxComponent.h" 
#include "GameFramework/CharacterMovementComponent.h" 

ABoss::ABoss()
{
 	PrimaryActorTick.bCanEverTick = true;

}

void ABoss::BeginPlay() 
{
	Super::BeginPlay(); 

	HandAttackCollider = FindComponentByClass<UBoxComponent>(); 
	HandAttackCollider->OnComponentBeginOverlap.AddDynamic(this, &ABoss::OnHandAttackOverlapBegin); 

	BossMesh = FindComponentByClass<USkeletalMeshComponent>(); 

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

	if (!IsPhase2 && CurHP <= MaxHP * 3 / 10.0f) { 
		IsPhase2 = true; 
		Blackboard->SetValueAsBool("IsPhase2", true); 
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, TEXT("Phase 2")); 
	}

	if (IsJumpAttacking) { 
		JumpAttackCheck(); 
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

void ABoss::SetAttackState(UShapeComponent* Collider, bool State)
{ 
	Collider->SetGenerateOverlapEvents(State); 
	SetActorLocation(GetActorLocation() + FVector(0.1f, 0, 0)); 
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, State ? TEXT("AttackStart") : TEXT("AttackEnd")); 
	TempMesh->SetMaterial(0, State ? M_Attacking : M_Default);
} 

FVector ABoss::GetPlayerAround(float Distance)
{
	return Player->GetActorLocation() - GetBossToPlayerDir() * Distance; 
} 

void ABoss::LaunchPlayer(FVector Dir, float Force)
{ 
	Player->LaunchCharacter(Dir * Force, false, false); 
}

FVector ABoss::GetFistSwingDir()
{ 
	return HandAttackCollider->GetForwardVector() + FVector(0, 0, 0.1f); 
}

FVector ABoss::GetShoulderDir()
{
	return (GetBossToPlayerDir() + FVector(0.0f, 0.0f, 0.1f)) * 1500.0f; 
}

void ABoss::SetAttackType(EAttackType Value)
{ 
	AttackType = Value; 
} 

void ABoss::OnHandAttackOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	auto a = AttackType;
	switch (AttackType) {
	case EAttackType::Attack1:
		Execute_Hit(Player, { 10, 0 });
		break;
	case EAttackType::Attack2:
		Execute_Hit(Player, { 10, 0 });
		break;
	case EAttackType::Attack3:
		Execute_Hit(Player, { 10, 0 });
		LaunchPlayer(GetFistSwingDir(), 2000.0f);
		break;
	case EAttackType::Attack4:
		Execute_Hit(Player, { 10, 0 }); 
		break;
	case EAttackType::Attack5:
		Player->AttachToComponent(BossMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, "CatchedPosition");

		if (auto AIC = Cast<AAIC_Boss>(GetController())) {
			AIC->ClearFocus(2);
		}

		Player->GetComponentByClass<UCharacterMovementComponent>()->GravityScale = 0.0f;
		Player->GetComponentByClass<UCharacterMovementComponent>()->MaxWalkSpeed = 0.0f;

		GetWorld()->GetTimerManager().SetTimer(CatchTimerHandle, FTimerDelegate::CreateLambda([&]() {
			Player->GetComponentByClass<UCharacterMovementComponent>()->GravityScale = 1.0f;
			Player->GetComponentByClass<UCharacterMovementComponent>()->MaxWalkSpeed = 600.0f;
			
			Player->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

			LaunchPlayer(GetBossToPlayerDir() - FVector(0.0f, 0.0f, 0.65f), 1500.0f);

			GetWorld()->GetTimerManager().SetTimer(ThrowTimerHandle, FTimerDelegate::CreateLambda([&]() {
				Execute_Hit(Player, { 10, 0 });

				GetWorld()->GetTimerManager().ClearTimer(ThrowTimerHandle); 
				}), 0.5f, false);

			GetWorld()->GetTimerManager().ClearTimer(CatchTimerHandle); 
			}), 0.75f, false);
		break;
	}
}

void ABoss::OnLightningExplosionAttackOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{ 

}

void ABoss::JumpAttackCheck()
{ 
	float Vel = GetVelocity().Length(); 
	if (Vel <= 0.1f && !IsExecuteJumpAttack) { 
		SetAttackState(HandAttackCollider, true); 
		IsExecuteJumpAttack = true; 

		GetWorld()->GetTimerManager().SetTimer(JumpAttackTimerHandle, FTimerDelegate::CreateLambda([&]() {
			SetAttackState(HandAttackCollider, false); 
			IsJumpAttacking = false; 
			IsExecuteJumpAttack = false; 

			GetWorld()->GetTimerManager().ClearTimer(JumpAttackTimerHandle); 
			}), 0.5f, false); 
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
