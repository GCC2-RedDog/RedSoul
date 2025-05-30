// Fill out your copyright notice in the Description page of Project Settings.


#include "AIC_Boss.h" 
#include "Boss.h" 
#include "BehaviorTree/BlackboardComponent.h" 

void AAIC_Boss::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn); 

	Boss = Cast<ABoss>(InPawn); 
}

void AAIC_Boss::SetBlackboard(ACharacter* OtherCharacter)
{
	Boss->Blackboard = GetBlackboardComponent();
	Blackboard->SetValueAsObject("Player", OtherCharacter); 
} 

void AAIC_Boss::Awaken(ACharacter* OtherCharacter)
{ 
	RunBehaviorTree(BT); 
	SetBlackboard(OtherCharacter); 
	
}

void AAIC_Boss::SetAttackCoolTime()
{ 
	Blackboard->SetValueAsBool("IsCoolTime", true); 
	GetWorld()->GetTimerManager().SetTimer(CoolTimeHandle, FTimerDelegate::CreateLambda([&]()
	{
		Blackboard->SetValueAsBool("IsCoolTime", false); 

		GetWorld()->GetTimerManager().ClearTimer(CoolTimeHandle); 
	}), 2.25f, false); 
}

