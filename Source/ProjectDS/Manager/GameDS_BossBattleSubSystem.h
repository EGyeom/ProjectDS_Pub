// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectDS/Manager/GameDS_BattleSubSystem.h"
#include "GameDS_BossBattleSubSystem.generated.h"
class AGameDS_Boss;

UCLASS()
class PROJECTDS_API UGameDS_BossBattleSubSystem : public UGameDS_BattleSubSystem
{
	GENERATED_BODY()
public:
	UGameDS_BossBattleSubSystem();
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	bool SetBoss(AGameDS_Character* InCharacter);
	AGameDS_Boss* GetBoss() { return BossCharacter; }
private:
	AGameDS_Boss* BossCharacter;
};
