// Fill out your copyright notice in the Description page of Project Settings.


#include "ANS_Catched.h"

void UANS_Catched::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
                               const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (Boss = Cast<ABoss>(MeshComp->GetOwner()))
	{ 
		Boss->PlayerCatch(); 
	}
}

void UANS_Catched::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (Boss)
	{
		Boss->PlayerThrow();
		Boss->SetBlockToPlayer(false); 
	}
}
