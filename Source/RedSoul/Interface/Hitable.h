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
	UFUNCTION() 
	virtual void Hit(FAttackInfo AttackInfo) = 0;

};
