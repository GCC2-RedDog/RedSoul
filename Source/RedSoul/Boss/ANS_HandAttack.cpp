// Fill out your copyright notice in the Description page of Project Settings.


#include "ANS_HandAttack.h"

void UANS_HandAttack::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
                                  const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (Boss = Cast<ABoss>(MeshComp->GetOwner()))
	{
		Boss->SetAttackState(true, true); 
	}
}

void UANS_HandAttack::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (Boss)
	{
		Boss->SetAttackState(true, false); 
	}
}
