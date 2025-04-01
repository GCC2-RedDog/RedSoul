// Fill out your copyright notice in the Description page of Project Settings.


#include "AI_Boss.h" 
#include "Boss.h" 

void UAI_Boss::NativeUpdateAnimation(float DeltaSeconds)
{ 
	if (auto* boss = Cast<ABoss>(GetOwningActor())) {
		Velocity = boss->GetVelocity().Length(); 
		ZVelocity = boss->GetVelocity().Z; 
		IsPhase2 = boss->IsPhase2;
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, IsPhase2 ? TEXT("True") : TEXT("False"));
	}
}
