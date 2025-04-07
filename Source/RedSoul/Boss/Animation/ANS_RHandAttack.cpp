// Fill out your copyright notice in the Description page of Project Settings.


#include "ANS_RHandAttack.h"

void UANS_RHandAttack::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
								   const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (Boss = Cast<ABoss>(MeshComp->GetOwner()))
	{
		Boss->SetAttackState(EAttackHand::AH_Right, true, true); 
	}
}

void UANS_RHandAttack::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (Boss)
	{
		Boss->SetAttackState(EAttackHand::AH_Right, true, false);
		Boss->SetBlockToPlayer(false); 
	}
}
