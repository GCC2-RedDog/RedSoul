// Fill out your copyright notice in the Description page of Project Settings.


#include "AI_Boss.h" 
#include "Boss.h" 

void UAI_Boss::NativeUpdateAnimation(float DeltaSeconds)
{ 
	if (auto* Boss = Cast<ABoss>(GetOwningActor())) {
		Velocity = Boss->GetVelocity().Length(); 

		IsActiveAttack2 = Boss->IsActiveAttack2; 
		IsFalling = !(Boss->GetVelocity().Z); 
		
		IsPhase2 = Boss->IsPhase2; 
	}
}
