// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "../Boss.h" 
#include "AI_Boss.generated.h"

UCLASS()
class REDSOUL_API UAI_Boss : public UAnimInstance
{
	GENERATED_BODY()
	
public: 
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
	UPROPERTY(BlueprintReadOnly) 
	float Speed; 
	UPROPERTY(BlueprintReadOnly)
	float Dir;
	UPROPERTY(BlueprintReadOnly)
	float FloorDist; 
	
	UPROPERTY(BlueprintReadOnly) 
	bool IsAwake; 
	UPROPERTY(BlueprintReadOnly) 
	bool IsActiveAttack2;
	UPROPERTY(BlueprintReadOnly) 
	bool IsAttack2Smash;
	UPROPERTY(BlueprintReadOnly) 
	bool IsActiveAttack5;
	UPROPERTY(BlueprintReadOnly) 
	bool IsAttack5Success;
	
	UPROPERTY(BlueprintReadOnly)
	bool IsFalling;
	UPROPERTY(BlueprintReadOnly) 
	bool IsPhase2; 
	UPROPERTY(BlueprintReadOnly) 
	bool IsDie; 

private: 
	TObjectPtr<ABoss> Boss; 
	
};
