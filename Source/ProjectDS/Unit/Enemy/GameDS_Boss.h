// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectDS/Unit/Enemy/GameDS_Enemy.h"
#include "GameDS_Boss.generated.h"

class AGameDS_FirstBossAIController;
class UGameDS_BossAnimInstance;

UCLASS()
class PROJECTDS_API AGameDS_Boss : public AGameDS_Enemy
{
	GENERATED_BODY()

public:
	AGameDS_Boss();
	virtual bool IsUnitType(EGameDS_UnitType InUnitType) override;
	virtual void UpdateHealthWidgetVisibleTime(float DeltaTime) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;
	virtual void SpawnInit(const FGameDS_UnitSpawnOption& InSpawnOption, const FGameDS_UnitStatInfo& InUnitStatInfo, const FGameDS_HeroStatInfo& InHeroStatInfo) override;

	virtual void OnDamaged(const FGameDS_DamageInfo& InDamageInfo) override;
	virtual void OnReset() override;

	uint32 GetPhase() const { return Phase; }
	bool IsPhaseEnd() const { return bPhaseEnd; }

	UFUNCTION(BlueprintCallable)
	void EndPhase(uint8 InPhase);

	UFUNCTION(BlueprintCallable)
	void ChangePhase(uint8 InPhase);

	UFUNCTION(BlueprintImplementableEvent)
	void SetPhaseInfo(uint8 InPhase);
private:
	AGameDS_FirstBossAIController* BossAIController;
	UGameDS_BossAnimInstance* BossAnimInstance;
	uint8 Phase;
	UActorComponent* Phase2Effect;
	bool bPhaseEnd;
};
