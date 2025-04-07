// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Hitable.generated.h" 

USTRUCT(BlueprintType)
struct FAttackInfo {
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage; 
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool Stun; 
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StunTime; 
	
};

UENUM(BlueprintType)
enum EAttackResult {
	AR_None		UMETA(DisplayName = "None"),
	AR_Parrying UMETA(DisplayName = "Parrying"),
	AR_Death	UMETA(DisplayName = "Death") 
}; 

UINTERFACE(MinimalAPI)
class UHitable : public UInterface
{
	GENERATED_BODY()
};

class REDSOUL_API IHitable
{
	GENERATED_BODY()

public: 
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent) 
	void Hit(FAttackInfo AttackInfo); 

};
