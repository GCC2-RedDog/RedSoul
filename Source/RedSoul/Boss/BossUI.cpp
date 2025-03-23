// Fill out your copyright notice in the Description page of Project Settings.


#include "BossUI.h" 
#include "Components/ProgressBar.h" 

void UBossUI::SetHPBar(float Ratio)
{ 
	HPBar->SetPercent(Ratio); 
}
