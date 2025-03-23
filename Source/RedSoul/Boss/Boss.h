// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../Interface/Hitable.h"
#include "Boss.generated.h"

UCLASS()
class REDSOUL_API ABoss : public ACharacter, public IHitable
{
	GENERATED_BODY()

public:
	ABoss();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	virtual void Hit(FAttackInfo AttackInfo) override; 

private: 
	UPROPERTY(EditAnywhere, Category = Stat)  
	float MaxHP; 
	UPROPERTY(EditAnywhere, Category = Stat)  
	float CurHP; 

};
