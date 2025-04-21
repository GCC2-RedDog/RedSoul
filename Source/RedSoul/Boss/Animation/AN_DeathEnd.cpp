// Fill out your copyright notice in the Description page of Project Settings.


#include "AN_DeathEnd.h" 
#include "../Boss.h" 

void UAN_DeathEnd::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	if (auto Boss = Cast<ABoss>(MeshComp->GetOwner()))
	{
		Boss->IsDieEnd = true; 
	}
}
