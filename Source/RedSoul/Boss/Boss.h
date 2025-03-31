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
	void SetAttackState(UShapeComponent* Collider, bool State);

	UFUNCTION(BlueprintCallable, BlueprintPure) 
	FVector GetPlayerAround(float Distance); 

	UFUNCTION(BlueprintCallable) 
	void LaunchPlayer(FVector Dir, float Force); 
	UFUNCTION(BlueprintCallable, BlueprintPure) 
	FVector GetFistSwingDir(); 
	UFUNCTION(BlueprintCallable, BlueprintPure) 
	FVector GetShoulderDir(); 

	UFUNCTION(BlueprintCallable) 
	void SetAttackType(EAttackType Value); 
	
	UPROPERTY()
	TObjectPtr<class UBlackboardComponent> Blackboard; 
	UPROPERTY() 
	TObjectPtr<ACharacter> Player; 

	UPROPERTY(BlueprintReadWrite) 
	bool IsJumpAttacking; 
	
	EAttackType AttackType; 

private: 
	UFUNCTION()
	void OnHandAttackOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult); 
	void OnLightningExplosionAttackOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult); 
	
	void JumpAttackCheck(); 
	
	void Die(); 

	FVector GetBossToPlayerDir(); 

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

	bool IsAwake; 
	bool IsPhase2; 

	bool IsExecuteJumpAttack; 

	FTimerHandle JumpAttackTimerHandle; 
	FTimerHandle CatchTimerHandle; 
	FTimerHandle ThrowTimerHandle; 

	UPROPERTY(EditAnywhere, Category = Materials) 
	TObjectPtr<UStaticMeshComponent> TempMesh; 
 	UPROPERTY(EditAnywhere, Category = Materials) 
	TObjectPtr<UMaterialInterface> M_AttackReady; 
	UPROPERTY(EditAnywhere, Category = Materials)
	TObjectPtr<UMaterialInterface> M_Attacking;
	UPROPERTY(EditAnywhere, Category = Materials)
	TObjectPtr<UMaterialInterface> M_Default;

};
