// Fill out your copyright notice in the Description page of Project Settings.


#include "ANS_Attack5.h" 
#include "../Boss.h" 

void UANS_Attack5::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
								   const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (Boss = Cast<ABoss>(MeshComp->GetOwner()))
	{
		Boss->IsActiveAttack5 = false; 
		Boss->SetAttackState(EAttackHand::AH_Left, true, true);
	}
}

void UANS_Attack5::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (Boss)
	{
		Boss->SetAttackState(EAttackHand::AH_Left, true, false); 
	}
}
