// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ProjectDS/Unit/GameDS_Character.h"
#include "ProjectDS/Unit/GameDS_UnitDefine.h"
#include "GameDS_Enemy.generated.h"

class UPawnSensingComponent;
class UGameDS_UnitInfoWidget;
class UWidgetComponent;
class UPrimitiveComponent;
class USphereComponent;
class UBehaviorTree;
class AGameDS_EnemyAIController;
class UGameDS_UnitAnimInstance;
class UGameDS_StatGaugeWidget;
class UGameDS_LockOnWidget;

DECLARE_MULTICAST_DELEGATE(FCharacterOnDespawnDelegate);

UCLASS()
class PROJECTDS_API AGameDS_Enemy : public AGameDS_Character
{
	GENERATED_BODY()
public:
	AGameDS_Enemy();

	FCharacterOnDespawnDelegate OnDespawnDelegate;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;
	virtual void SpawnInit(const FGameDS_UnitSpawnOption& InSpawnOption, const FGameDS_UnitStatInfo& InUnitStatInfo, const FGameDS_HeroStatInfo& InHeroStatInfo) override;

	virtual bool IsUnitType(EGameDS_UnitType InUnitType) override;
	virtual void OnDamaged(const FGameDS_DamageInfo& InDamageInfo) override;
	virtual void UpdateHealthWidgetVisible(bool bInVisible);
	virtual void UpdateHealthWidgetVisibleTime(float DeltaTime);

	virtual void OnReset() override;

	UFUNCTION()
	void OnSeePawn(APawn* InPawn);

	void SetInitialPosition();

	UFUNCTION(BlueprintCallable)
	AGameDS_Character* GetTarget() const { return TargetCharacter; }

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void InitEnemyWaterSimV2(UObject* WaterSimBP);

	UGameDS_StatGaugeWidget* GetHealthGaugeWidget() const { return HealthGaugeWidget; }
	UGameDS_UnitInfoWidget* GetUnitInfoWidget() const { return UnitInfoWidget; }

	void ToggleLockOnImage();

	FName GetLockOnSocketName() const { return LockOnSocket; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Combat, meta = (AllowPrivateAccess = "true"))
	USphereComponent* AgroSphere;

	UPROPERTY(EditAnywhere, Category = "Behavior Tree", meta = (AllowPrivateAccess = "true"))
	UBehaviorTree* BehaviorTree;

	UPROPERTY()
	AGameDS_EnemyAIController* AIController;
	UPROPERTY()
	UGameDS_UnitAnimInstance* AnimInstance;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess="true"))
	UWidgetComponent* HealthWidget;
	UPROPERTY()
	UGameDS_StatGaugeWidget* HealthGaugeWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess="true"))
	UPawnSensingComponent* PawnSensing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess="true"))
	UWidgetComponent* DebugUnitInfoWidget;
	UPROPERTY()
	UGameDS_UnitInfoWidget* UnitInfoWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* LockOnWidget;

	UPROPERTY()
	UGameDS_LockOnWidget* LockOnImageWidget;

	bool bFindTarget = false;
	bool bBattleMode = false;
	AGameDS_Character* TargetCharacter = nullptr;

	TArray<FVector> SplinePathPoint;

	bool bHealthWidgetVisible = false;
	float HealthWidgetVisibleTime = 0.0f;

	FVector SpawnLocation = FVector::ZeroVector;
	FRotator SpawnRotator = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	FName LockOnSocket = TEXT("Chest");
};
