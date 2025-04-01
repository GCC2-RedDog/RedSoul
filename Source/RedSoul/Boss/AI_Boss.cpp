// Fill out your copyright notice in the Description page of Project Settings.


#include "AI_Boss.h" 
#include "Boss.h" 

void UAI_Boss::NativeUpdateAnimation(float DeltaSeconds)
{ 
	if (auto* Boss = Cast<ABoss>(GetOwningActor())) {
		Velocity = Boss->GetVelocity().Length(); 
		ZVelocity = Boss->GetVelocity().Z; 
		IsPhase2 = Boss->IsPhase2; 
		
		if (IsActiveAttack2) {
			IsActiveAttack2 = !(ZVelocity == 0); 
		}

		if (Boss->AttackType == EAttackType::Attack2) {
			IsActiveAttack2 = true; 
		} 
	}
}
