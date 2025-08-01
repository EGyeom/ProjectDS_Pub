// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectDS/Unit/Enemy/GameDS_BossAnimInstance.h"
#include "ProjectDS/Manager/GameDS_DataManager.h"
#include "ProjectDS/Unit/Enemy/GameDS_Boss.h"
#include "GameDS_BossAnimInstance.h"
#include "ProjectDS/Manager/GameDS_UIManager.h"

UGameDS_BossAnimInstance::UGameDS_BossAnimInstance()
{
	bRun = false;
	OwnerBoss = Cast<AGameDS_Boss>(OwnerCharacter);
}

void UGameDS_BossAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
}

void UGameDS_BossAnimInstance::InitAnimationAlias()
{	
	if (OwnerCharacter == nullptr)
		return;
	
	if (OwnerBoss == nullptr)
	{
		OwnerBoss = Cast<AGameDS_Boss>(OwnerCharacter);
		if (OwnerBoss == nullptr)
			return;
	}
	
	Super::InitAnimationAlias();
}
void UGameDS_BossAnimInstance::EndDieAnimCallBack()
{
	if (UGameDS_UIManager* UIManager = UGameInstance::GetSubsystem<UGameDS_UIManager>(OwnerCharacter->GetGameInstance()))
	{
		UIManager->RemoveInfoUI(EGameDS_InfoUIType::BossUI);
	}
	
	Super::EndDieAnimCallBack();
}
