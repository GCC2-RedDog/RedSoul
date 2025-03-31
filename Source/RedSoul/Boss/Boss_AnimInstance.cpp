// Fill out your copyright notice in the Description page of Project Settings.


#include "Boss_AnimInstance.h" 
#include "Boss.h" 

void UBoss_AnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{ 
	if (auto* boss = Cast<ABoss>(GetOwningActor())) {
		Velocity = boss->GetVelocity().Length(); 
		ZVelocity = boss->GetVelocity().Z; 
		AttackType = boss->AttackType;
	}
}
