// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BossUI.generated.h"

UCLASS()
class REDSOUL_API UBossUI : public UUserWidget
{
	GENERATED_BODY()
	
public: 
	void SetHPBar(float Ratio); 

private: 
	UPROPERTY(meta = (BindWidget)) 
	class UProgressBar* HPBar; 

};
