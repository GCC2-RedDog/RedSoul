// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AIC_Boss.generated.h"

UCLASS()
class REDSOUL_API AAIC_Boss : public AAIController
{
	GENERATED_BODY() 

public: 
	virtual void OnPossess(APawn* InPawn) override; 
	 
	void Awaken(ACharacter* OtherCharacter); 

	UFUNCTION(BlueprintCallable) 
	void SetAttackCoolTime(); 
	
	UPROPERTY(EditAnywhere, Category = AI)
	TObjectPtr<UBehaviorTree> BT; 

private: 
	void SetBlackboard(ACharacter* OtherCharacter); 

	UPROPERTY() 
	TObjectPtr<class ABoss> Boss; 

	FTimerHandle CoolTimeHandle; 

};
