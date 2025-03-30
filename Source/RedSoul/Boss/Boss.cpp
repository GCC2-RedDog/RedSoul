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

	AttackCollider = FindComponentByClass<UBoxComponent>(); 

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

void ABoss::SetAttackState(bool State)
{ 
	AttackCollider->SetGenerateOverlapEvents(State); 
} 

FVector ABoss::GetPlayerAround(float Distance)
{
	return Player->GetActorLocation() - GetBossToPlayerDir() * Distance; 
} 

void ABoss::CatchPlayer(FName SocketName)
{ 
	Player->AttachToComponent(BossMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName); 
	
	if (auto AIC = Cast<AAIC_Boss>(GetController())) {
		AIC->ClearFocus(2); 
	} 

	Player->GetComponentByClass<UCharacterMovementComponent>()->GravityScale = 0.0f; 
	Player->GetComponentByClass<UCharacterMovementComponent>()->MaxWalkSpeed = 0.0f; 
}

void ABoss::ReleasePlayer()
{ 
	Player->GetComponentByClass<UCharacterMovementComponent>()->GravityScale = 1.0f; 
	Player->GetComponentByClass<UCharacterMovementComponent>()->MaxWalkSpeed = 500.0f;

	Player->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform); 

	LaunchPlayer(GetCatchThrowDir(), 1500.0f); 
}

void ABoss::LaunchPlayer(FVector Dir, float Force)
{ 
	Player->LaunchCharacter(Dir * Force, false, false); 
}

FVector ABoss::GetFistSwingDir()
{ 
	return AttackCollider->GetForwardVector() + FVector(0, 0, 0.1f); 
}

FVector ABoss::GetShoulderDir()
{
	return (GetBossToPlayerDir() + FVector(0.0f, 0.0f, 0.1f)) * 1500.0f; 
}

FVector ABoss::GetCatchThrowDir()
{ 
	return GetBossToPlayerDir() - FVector(0.0f, 0.0f, 0.65f); 
}

FVector ABoss::GetBossToPlayerDir()
{
	FVector Dir = Player->GetActorLocation() - GetActorLocation(); 
	Dir.Z = 0.0f; 
	Dir.Normalize();

	return Dir;
}

void ABoss::Die()
{ 
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, TEXT("Boss Die")); 

	Destroy(); 
} 
