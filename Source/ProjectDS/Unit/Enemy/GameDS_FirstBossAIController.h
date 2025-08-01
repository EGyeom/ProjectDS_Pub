// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectDS/Unit/Enemy/GameDS_EnemyAIController.h"
#include "GameDS_FirstBossAIController.generated.h"

class AGameDS_Boss;
class UGameDS_BossAnimInstance;
/**
 * 
 */
UCLASS()
class PROJECTDS_API AGameDS_FirstBossAIController : public AGameDS_EnemyAIController
{
	GENERATED_BODY()

	AGameDS_FirstBossAIController();

public:
	virtual void Init(AGameDS_Enemy* InEnemy, UGameDS_UnitAnimInstance* InAnimInstance) override;
	virtual void LastInit() override;
	virtual void InitPatrolPath(const TArray<FVector>& SplinePath) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void UpdateAction(float DeltaSeconds) override;
	virtual void UpdateRotateValue(float DeltaSeconds) override;

	virtual void ChangeBattleModeSetting(bool bActive, AGameDS_HeroCharacter* InTarget = nullptr) override;

	UPROPERTY()
	AGameDS_Boss* OwnerBoss;

	UPROPERTY()
	UGameDS_BossAnimInstance* OwnerAnimInstance;

	void SetUseSkill(bool InUse);
	void ResetBoss(bool IsInteraction);
private:
	float AttackDelay;
	float TickCount;
	bool bUsedSkill;
	bool bInteraction;
};
