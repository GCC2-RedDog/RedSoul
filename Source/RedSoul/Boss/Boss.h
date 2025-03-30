// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../Interface/Hitable.h" 
#include "../Interface/Interactive.h" 
#include "Boss.generated.h" 

UCLASS()
class REDSOUL_API ABoss : public ACharacter, public IHitable, public IInteractive 
{
	GENERATED_BODY()

public:
	ABoss();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override; 

	virtual void Hit_Implementation(FAttackInfo AttackInfo) override; 
	virtual void Interaction_Implementation(ACharacter* OtherCharacter) override;

	UFUNCTION(BlueprintCallable) 
	void SetAttackState(bool State);

	UFUNCTION(BlueprintCallable, BlueprintPure) 
	FVector GetPlayerAround(float Distance); 

	UFUNCTION(BlueprintCallable) 
	void CatchPlayer(FName SocketName); 
	UFUNCTION(BlueprintCallable) 
	void ReleasePlayer(); 

	UFUNCTION(BlueprintCallable) 
	void LaunchPlayer(FVector Dir, float Force); 
	UFUNCTION(BlueprintCallable, BlueprintPure) 
	FVector GetFistSwingDir(); 
	UFUNCTION(BlueprintCallable, BlueprintPure) 
	FVector GetShoulderDir(); 
	
	UPROPERTY()
	TObjectPtr<class UBlackboardComponent> Blackboard; 
	UPROPERTY() 
	TObjectPtr<ACharacter> Player; 

private: 
	FVector GetBossToPlayerDir(); 
	FVector GetCatchThrowDir();  

	void Die(); 

	UPROPERTY(EditAnywhere, Category = Stat)  
	float MaxHP; 
	UPROPERTY(EditAnywhere, Category = Stat)  
	float CurHP; 

	UPROPERTY()
	TObjectPtr<USkeletalMeshComponent> BossMesh;

	UPROPERTY(EditAnywhere, Category = Widget) 
	TSubclassOf<class UBossUI> BossInfoWidget; 
	UPROPERTY()
	TObjectPtr<UUserWidget> BossInfoObject; 

	UPROPERTY() 
	TObjectPtr<class UBoxComponent> AttackCollider; 

	bool IsAwake; 
	bool IsPhase2; 
	
};
