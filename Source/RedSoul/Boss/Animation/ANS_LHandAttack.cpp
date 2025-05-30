// Fill out your copyright notice in the Description page of Project Settings.


#include "ANS_LHandAttack.h" 
#include "../Boss.h" 

void UANS_LHandAttack::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
                                   const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (Boss = Cast<ABoss>(MeshComp->GetOwner()))
	{
		Boss->SetAttackState(EAttackHand::AH_Left, true, true); 
	}
}

void UANS_LHandAttack::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (Boss)
	{
		Boss->SetAttackState(EAttackHand::AH_Left, true, false); 
		Boss->SetIgnoreToPlayer(false); 
	}
}
