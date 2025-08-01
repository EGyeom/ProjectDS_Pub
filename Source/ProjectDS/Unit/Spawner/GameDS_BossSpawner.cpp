// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectDS/Unit/Spawner/GameDS_BossSpawner.h"
#include "ProjectDS/Manager/GameDS_UnitManager.h"
#include "Components/CapsuleComponent.h"
#include "Engine/GameInstance.h"

AGameDS_BossSpawner::AGameDS_BossSpawner()
{
	SavedSpawnOption = FGameDS_UnitSpawnOption();
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CollisionCapsule"));
	CapsuleComponent->ShapeColor = FColor(255, 138, 5, 255);
	CapsuleComponent->bDrawOnlyIfSelected = true;
	CapsuleComponent->InitCapsuleSize(50.0f, 50.0f);
	CapsuleComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	CapsuleComponent->bShouldCollideWhenPlacing = true;
	CapsuleComponent->SetShouldUpdatePhysicsVolume(true);
	CapsuleComponent->Mobility = EComponentMobility::Static;
	RootComponent = CapsuleComponent;

	bSpawned = false;
}

void AGameDS_BossSpawner::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AGameDS_BossSpawner::BeginPlay()
{
	if (UGameDS_UnitManager* UnitManager = UGameInstance::GetSubsystem<UGameDS_UnitManager>(GetGameInstance()))
	{
		SavedSpawnOption.UnitType = UnitType;
		SavedSpawnOption.UnitID = UnitManager->GetNextUnitID();
		SavedSpawnOption.DataID = DataID;
		SavedSpawnOption.Location = GetActorLocation();
		SavedSpawnOption.Rotator = GetActorRotation();
		SavedSpawnOption.bBattleMode = bBattleMode;
	}
}

bool AGameDS_BossSpawner::SpawnBoss()
{
	if (bSpawned == true)
		return false;

	if (UGameDS_UnitManager* UnitManager = UGameInstance::GetSubsystem<UGameDS_UnitManager>(GetGameInstance()))
	{
		UnitManager->SpawnCharacter(SavedSpawnOption);
		bSpawned = true;
	}

	return true;
}
