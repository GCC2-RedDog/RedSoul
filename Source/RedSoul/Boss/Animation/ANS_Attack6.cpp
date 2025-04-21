// Fill out your copyright notice in the Description page of Project Settings.


#include "ANS_Attack6.h" 
#include "../Boss.h" 
#include "NiagaraComponent.h" 

void UANS_Attack6::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
								  const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (Boss = Cast<ABoss>(MeshComp->GetOwner()))
	{ 
		Boss->SetAttackState(AH_None, false, true);
		Boss->NS_LightningExplosion->Activate();
	}
}

void UANS_Attack6::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (Boss)
	{ 
		Boss->SetAttackState(AH_None, false, false); 
		Boss->NS_LightningExplosion->Deactivate(); 
	}
}
