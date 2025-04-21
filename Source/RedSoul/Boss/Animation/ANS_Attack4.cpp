// Fill out your copyright notice in the Description page of Project Settings.


#include "ANS_Attack4.h"
#include "../Boss.h" 

void UANS_Attack4::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
								  const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (Boss = Cast<ABoss>(MeshComp->GetOwner()))
	{
		MeshComp->GetAnimInstance()->RootMotionMode = ERootMotionMode::IgnoreRootMotion; 

		FVector Dir = Boss->GetShoulderDir(); 
		Boss->SetActorRotation(Dir.Rotation()); 
		Boss->LaunchCharacter(Dir, false, false);
	}
} 

void UANS_Attack4::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (Boss)
	{
		MeshComp->GetAnimInstance()->RootMotionMode = ERootMotionMode::RootMotionFromEverything;
		Boss->SetAttackState(EAttackHand::AH_Left, true, false);
	}
}
