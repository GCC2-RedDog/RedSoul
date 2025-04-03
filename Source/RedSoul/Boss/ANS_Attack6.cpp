// Fill out your copyright notice in the Description page of Project Settings.


#include "ANS_Attack6.h" 

void UANS_Attack6::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
								  const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (Boss = Cast<ABoss>(MeshComp->GetOwner()))
	{
		Boss->LightningExplosionMesh->SetVisibility(true); 
		Boss->SetAttackState(false, true); 
	}
}

void UANS_Attack6::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (Boss)
	{
		Boss->LightningExplosionMesh->SetVisibility(false); 
		Boss->SetAttackState(false, false); 
	}
}
