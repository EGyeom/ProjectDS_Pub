// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectDS/Manager/GameDS_BossBattleSubSystem.h"
#include "ProjectDS/Unit/Enemy/GameDS_Boss.h"

UGameDS_BossBattleSubSystem::UGameDS_BossBattleSubSystem()
{
}

void UGameDS_BossBattleSubSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UGameDS_BossBattleSubSystem::Deinitialize()
{
	Super::Deinitialize();
}

bool UGameDS_BossBattleSubSystem::SetBoss(AGameDS_Character* InCharacter)
{
	if (InCharacter == nullptr)
		return false;

	BossCharacter = Cast<AGameDS_Boss>(InCharacter);
	
	if (BossCharacter == nullptr)
		return false;

	return true;
}
