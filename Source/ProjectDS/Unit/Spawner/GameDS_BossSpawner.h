// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "ProjectDS/Unit/Spawner/GameDS_UnitSpawner.h"
#include "GameDS_BossSpawner.generated.h"

UCLASS()
class PROJECTDS_API AGameDS_BossSpawner : public AGameDS_UnitSpawner
{
	GENERATED_BODY()

	AGameDS_BossSpawner();

	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable)
	bool SpawnBoss();

	UFUNCTION(BlueprintCallable)
	void SetSpawnedValue(bool InSpawnd) { bSpawned = InSpawnd; }

	UPROPERTY()
	FGameDS_UnitSpawnOption SavedSpawnOption;

	UPROPERTY()
	TObjectPtr<class UCapsuleComponent> CapsuleComponent;

	UPROPERTY()
	bool bSpawned;
};
