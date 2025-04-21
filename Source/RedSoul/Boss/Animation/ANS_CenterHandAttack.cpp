// Fill out your copyright notice in the Description page of Project Settings.


#include "ANS_CenterHandAttack.h"
#include "../Boss.h" 

void UANS_CenterHandAttack::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
                               const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (Boss = Cast<ABoss>(MeshComp->GetOwner()))
	{
		Boss->SetAttackState(EAttackHand::AH_Center, true, true); 
	}
}

void UANS_CenterHandAttack::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (Boss)
	{
		Boss->SetAttackState(EAttackHand::AH_Center, true, false); 
		Boss->SetIgnoreToPlayer(false);
	}
}
