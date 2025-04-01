// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "AI_Boss.generated.h"

UCLASS()
class REDSOUL_API UAI_Boss : public UAnimInstance
{
	GENERATED_BODY()
	
public: 
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UPROPERTY(BlueprintReadOnly) 
	float Velocity; 
	UPROPERTY(BlueprintReadOnly)
	float ZVelocity;
	UPROPERTY(BlueprintReadOnly) 
	bool IsPhase2; 
	
};
