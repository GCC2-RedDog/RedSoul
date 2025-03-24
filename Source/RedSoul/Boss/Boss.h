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

	virtual void Hit_Implementation(FAttackInfo AttackInfo) override; 

	UFUNCTION(BlueprintCallable) 
	void Awaken(); 

	UFUNCTION(BlueprintCallable) 
	void SetAttackState(bool State); 

	UPROPERTY(EditAnywhere)
	TObjectPtr<class UBlackboardComponent> Blackboard; 

private: 
	UPROPERTY(EditAnywhere, Category = Stat)  
	float MaxHP; 
	UPROPERTY(EditAnywhere, Category = Stat)  
	float CurHP; 

	UPROPERTY(EditAnywhere, Category = Widget) 
	TSubclassOf<class UBossUI> BossInfoWidget; 
	
	UPROPERTY()
	TObjectPtr<UUserWidget> BossInfoObject; 


	UPROPERTY() 
	TObjectPtr<class UBoxComponent> AttackCollider; 

};
