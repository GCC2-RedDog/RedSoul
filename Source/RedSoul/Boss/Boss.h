// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../Interface/Hitable.h" 
#include "../Interface/Interactive.h" 
#include "Boss.generated.h" 

UENUM(BlueprintType) 
enum EAttackType { 
	None	 UMETA(DisplayName = "None"), 
	Attack1	 UMETA(DisplayName = "Attack1"), 
	Attack2	 UMETA(DisplayName = "Attack2"), 
	Attack3	 UMETA(DisplayName = "Attack3"), 
	Attack4	 UMETA(DisplayName = "Attack4"), 
	Attack5	 UMETA(DisplayName = "Attack5"), 
	Attack6	 UMETA(DisplayName = "Attack6") 
};

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
	void Attack(EAttackType Value); 
	UFUNCTION(BlueprintCallable) 
	void SetAttackState(bool IsHandAttack, bool State);

	UFUNCTION(BlueprintCallable, BlueprintPure) 
	FVector GetPlayerAround(float Distance); 

	UFUNCTION(BlueprintCallable) 
	void LaunchPlayer(FVector Dir, float Force); 
	UPROPERTY()
	TObjectPtr<class UBlackboardComponent> Blackboard; 
	UPROPERTY() 
	TObjectPtr<ACharacter> Player; 
	
	bool IsAwake; 
	bool IsActiveAttack2; 
	bool IsPhase2;
	bool IsDie;
	
	bool IsTurnLeft; 
	float DeltaRotAngle;
	FVector PreForward;

	FVector GetShoulderDir();

	UPROPERTY(EditAnywhere, Category = Temp)
	TObjectPtr<UStaticMeshComponent> LightningExplosionMesh;
	
private: 
	UFUNCTION()
	void OnHandAttackOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult); 
	UFUNCTION()
	void OnLightningExplosionAttackOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult); 
	
	void Die(); 

	FVector GetBossToPlayerDir(); 
	FVector GetFistSwingDir(); 
	FVector GetThrowPlayerDir(); 
	FVector GetShoulderHitDir(); 

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
	TObjectPtr<class UBoxComponent> HandAttackCollider; 
	UPROPERTY()
	TObjectPtr<class USphereComponent> LightningExplosionAttackCollider; 

	EAttackType AttackType;
	
	FTimerHandle AwakeTimerHandle;
	
	FTimerHandle Attack2TimerHandle; 
	FTimerHandle Attack4TimerHandle; 
	FTimerHandle Attack5TimerHandle; 
	FTimerHandle Attack6TimerHandle; 
	
	FTimerHandle CatchTimerHandle; 
	FTimerHandle ThrowTimerHandle;

	UPROPERTY(EditAnywhere, Category = Montages)
	TObjectPtr<UAnimMontage> Awake_Montage;
	UPROPERTY(EditAnywhere, Category = Montages)
	TObjectPtr<UAnimMontage> Attack1_Montage;
	UPROPERTY(EditAnywhere, Category = Montages)
	TObjectPtr<UAnimMontage> Attack3_Montage;
	UPROPERTY(EditAnywhere, Category = Montages)
	TObjectPtr<UAnimMontage> Attack4_Montage;
	UPROPERTY(EditAnywhere, Category = Montages)
	TObjectPtr<UAnimMontage> Attack6_Montage;
	UPROPERTY(EditAnywhere, Category = Montages) 
	TObjectPtr<UAnimMontage> Hit_Montage; 
};
