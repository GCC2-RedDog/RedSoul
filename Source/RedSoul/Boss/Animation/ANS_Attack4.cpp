// Fill out your copyright notice in the Description page of Project Settings.


#include "ANS_Attack4.h"
#include "../Boss.h"

void UANS_Attack4::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
								  const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (auto Boss = Cast<ABoss>(MeshComp->GetOwner()))
	{
		Boss->LaunchCharacter(Boss->GetShoulderDir(), false, false); 
	}
}