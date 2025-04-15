// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../Interface/Hitable.h" 
#include "../Interface/Interactive.h" 
#include "Boss.generated.h" 

UENUM(BlueprintType) 
enum EAttackType { 
	AT_None	 UMETA(DisplayName = "None"), 
	AT_Attack1	 UMETA(DisplayName = "Attack1"), 
	AT_Attack2	 UMETA(DisplayName = "Attack2"), 
	AT_Attack3	 UMETA(DisplayName = "Attack3"), 
	AT_Attack4	 UMETA(DisplayName = "Attack4"), 
	AT_Attack5	 UMETA(DisplayName = "Attack5"), 
	AT_Attack6	 UMETA(DisplayName = "Attack6") 
};

UENUM(BlueprintType) 
enum EAttackHand { 
	AH_None		UMETA(DisplayName = "None"), 
	AH_Center	UMETA(DisplayName = "Center"),
	AH_Left		UMETA(DisplayName = "Left"), 
	AH_Right	UMETA(DisplayName = "Right") 
}; 

UCLASS()
class REDSOUL_API ABoss : public ACharacter, public IHitable, public IInteractive 
{
	GENERATED_BODY()

public:
	ABoss();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override; 

	virtual EAttackResult Hit_Implementation(FAttackInfo AttackInfo) override; 
	virtual void Interaction_Implementation(ACharacter* OtherCharacter) override;

	UFUNCTION(BlueprintCallable)
	void Attack(EAttackType Value); 
	UFUNCTION(BlueprintCallable) 
	void SetAttackState(EAttackHand Hand, bool IsHandAttack, bool State); 
	void SetIgnoreToPlayer(bool State); 
	UFUNCTION(BlueprintCallable) 
	void FocusToPlayer(); 
	
	void PlayerCatch(); 
	void PlayerThrow(); 

	void CheckDirectHit(); 
	
	UFUNCTION(BlueprintCallable, BlueprintPure) 
	FVector GetPlayerAround(float Distance); 
	FHitResult GetHitResult(FVector AttackColliderLocation, FVector HitActorLocation); 

	FVector GetShoulderDir(); 
	
	UPROPERTY()
	TObjectPtr<class UBlackboardComponent> Blackboard;
	
	bool IsAwake; 
	bool IsActiveAttack2;
	bool IsAttack2Smash; 
	bool IsActiveAttack5;
	bool IsAttack5Success; 
	bool IsPhase2; 
	bool IsDie; 
	bool IsParryed; 
	
	UPROPERTY(EditAnywhere, Category = VFX)
	TObjectPtr<class UNiagaraComponent> NS_LightningExplosion; 

private: 
	UFUNCTION()
	void OnHandAttackOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult); 
	UFUNCTION()
	void OnLightningExplosionAttackOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnMeshOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult); 

	void LaunchPlayer(FVector Dir, float Force); 
	void Die();

	void StopLogic(); 

	void SetHandCollider(FString Socket, FVector Scale); 
	void PlayMontage(UAnimMontage* Montage); 
	
	FVector GetBossToPlayerDir(); 
	FVector GetFistSwingDir(); 
	FVector GetThrowPlayerDir(); 
	FVector GetShoulderHitDir(); 

	UPROPERTY(EditAnywhere, Category = Stat)  
	float MaxHP; 
	float CurHP; 
	
	UPROPERTY() 
	TObjectPtr<ACharacter> Player; 

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
	UPROPERTY()
	TObjectPtr<class UBoxComponent> DirectHitCollider; 
	
	FTimerHandle AwakeTimerHandle; 
	FTimerHandle Attack4TimerHandle; 
	FTimerHandle Attack5TimerHandle; 
	FTimerHandle Attack6TimerHandle; 
	FTimerHandle ThrowTimerHandle; 
	FTimerHandle FocusTimerHandle; 
	FTimerHandle HitTimerHandle; 
	FTimerHandle ParryedTimerHandle; 
	FTimerHandle Phase2TimerHandle;
	FTimerHandle AttackTimerHandle; 

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
	UPROPERTY(EditAnywhere, Category = Montages)
	TObjectPtr<UAnimMontage> LTurn_Montage;
	UPROPERTY(EditAnywhere, Category = Montages)
	TObjectPtr<UAnimMontage> RTurn_Montage; 
	UPROPERTY(EditAnywhere, Category = Montages)
	TObjectPtr<UAnimMontage> Parryed_Montage;
	UPROPERTY(EditAnywhere, Category = Montages)
	TObjectPtr<UAnimMontage> Phase2_Montage;
	
	UPROPERTY(EditAnywhere, Category=VFX) 
	TObjectPtr<class UNiagaraSystem> NS_StoneParts;
	UPROPERTY(EditAnywhere, Category=VFX) 
	TObjectPtr<class UNiagaraComponent> NS_LightningAura_L;
	UPROPERTY(EditAnywhere, Category=VFX) 
	TObjectPtr<class UNiagaraComponent> NS_LightningAura_R; 
	
	EAttackType AttackType;

	bool IsFocusToPlayer; 
	bool IsHit;
	
	bool IsOverlapHand;
	bool IsOverlapMesh; 

	float FocusToPlayerAngle; 
	float BossToPlayerDist;

	FVector Attack2TargetLocation; 
};
