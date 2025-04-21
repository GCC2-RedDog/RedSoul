// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactive.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UInteractive : public UInterface
{
	GENERATED_BODY()
};

class REDSOUL_API IInteractive
{
	GENERATED_BODY()
	
public: 
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent) 
	void Interaction(ACharacter* OtherCharacter); 

};
