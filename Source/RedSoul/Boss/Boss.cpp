// Fill out your copyright notice in the Description page of Project Settings.


#include "Boss.h" 
#include "BossUI.h" 
#include "AIC_Boss.h" 
#include "BehaviorTree/BlackboardComponent.h" 
#include "Components/BoxComponent.h" 

ABoss::ABoss()
{
 	PrimaryActorTick.bCanEverTick = true;

}

void ABoss::BeginPlay() 
{
	Super::BeginPlay(); 

	AttackCollider = FindComponentByClass<UBoxComponent>(); 

	BossInfoObject = CreateWidget<UBossUI>(GetWorld(), BossInfoWidget); 
	BossInfoObject->AddToViewport(); 
	Cast<UBossUI>(BossInfoObject)->SetHPBar(CurHP / MaxHP); 
}

void ABoss::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime); 

}

void ABoss::Hit_Implementation(FAttackInfo AttackInfo)
{ 
	CurHP -= AttackInfo.Damage; 
	Cast<UBossUI>(BossInfoObject)->SetHPBar(CurHP / MaxHP); 
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, FString::Printf(TEXT("Boss Hit %f"), CurHP)); 

	if (CurHP <= 0) {
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, TEXT("Boss Die")); 
	}
}

void ABoss::Awaken()
{ 
	Blackboard = Cast<AAIController>(GetController())->GetBlackboardComponent();
}

void ABoss::SetAttackState(bool State)
{ 
	Blackboard->SetValueAsBool("IsAttacking", State); 
	AttackCollider->SetGenerateOverlapEvents(State); 
}
