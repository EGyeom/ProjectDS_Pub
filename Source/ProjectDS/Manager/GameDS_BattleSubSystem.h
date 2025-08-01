// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ProjectDS/Manager/GameDS_CustomDatas.h"
#include "GameDS_BattleSubSystem.generated.h"

struct FGameDS_CollisionInfoDataTable;
class AGameDS_Character;

USTRUCT(BlueprintType)
struct FGameDS_DetailCollisionInfo
{
	GENERATED_BODY()

	const FGameDS_CollisionInfoDataTable* CollisionInfoData;
	FCollisionShape CollisionShape;
	EGameDS_DamageType DamageType = EGameDS_DamageType::None;

	float DamageValue = 0.0f;
	bool bHeroSkill = false;
	bool bNeutral = false;
	bool bValidData = false;
	bool bSummon = false;
	TArray<int32> CrowdControlIDList;
	
	UPROPERTY()
	AGameDS_Character* OwnerCharacter;
};

UCLASS(Config = Game)
class PROJECTDS_API UGameDS_BattleSubSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	UGameDS_BattleSubSystem();
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	FGameDS_DetailCollisionInfo MakeSkillCollision(const int32 SkillInfoID, AGameDS_Character* OwnerCharacter, bool bSummon, bool bNeutral) const;
	bool OnAttackCheck(const FGameDS_DetailCollisionInfo& DetailCollisionInfo, FVector NormalDirection = FVector::ZeroVector, FVector Location = FVector::ZeroVector, bool bHitCharacter = true);

	FCollisionShape MakeCollisionShape(const FGameDS_CollisionInfoDataTable* InCollisionInfoData) const;
	
	void SpawnSummonActor(int32 SummonID, FName SocketName, AGameDS_Character* OwnerCharacter, FVector Scale3D = FVector::OneVector);
	void SpawnSummonActor(int32 SummonID, FName SocketName, AGameDS_Character* OwnerCharacter, float InDelay = 0.0f);

	void SpawnSummonActorWithAngle(int32 SummonID, FName SocketName, AGameDS_Character* OwnerCharacter, float InAngle);

	AGameDS_Character* GetTargetCharacter(AGameDS_Character* Caster);

	UFUNCTION(BlueprintCallable)
	int CalculrateSpawnAngle(const int32 SummonID, const float InSkillAngle);
	
	UFUNCTION(BlueprintCallable)
	float GetSummonAngleFromDataTable(const int32 SummonID);
};
