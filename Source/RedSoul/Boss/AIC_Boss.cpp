// Fill out your copyright notice in the Description page of Project Settings.


#include "AIC_Boss.h" 
#include "Boss.h" 

void AAIC_Boss::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn); 

	Boss = Cast<ABoss>(InPawn); 
}

void AAIC_Boss::SetBlackboard(ACharacter* OtherCharacter)
{
	Boss->Blackboard = GetBlackboardComponent(); 
	Boss->Player = OtherCharacter; 
} 
