// Fill out your copyright notice in the Description page of Project Settings.


#include "AI_Boss.h" 

void UAI_Boss::NativeUpdateAnimation(float DeltaSeconds)
{ 
	if (!Boss) Boss = Cast<ABoss>(GetOwningActor()); 

	if (Boss) { 
		FVector Velocity =  Boss->GetVelocity(); 
		Dir = CalculateDirection(Velocity, Boss->GetActorRotation()); 
		IsAttack2Smash = !Velocity.Z;  

		Velocity.Z = 0;  
		Speed = Velocity.Length(); 
		
		IsAwake = Boss->IsAwake; 
		IsActiveAttack2 = Boss->IsActiveAttack2;
		IsActiveAttack5 = Boss->IsActiveAttack5;
		IsAttack5Success = Boss->IsAttack5Success; 
		
		IsPhase2 = Boss->IsPhase2; 
		IsDie = Boss->IsDie; 
	}
}
