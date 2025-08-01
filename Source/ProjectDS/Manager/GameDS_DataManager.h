#pragma once
#include "GameDS_CustomDatas.h"
#include "ProjectDS/Unit/GameDS_UnitDefine.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Templates/Casts.h"
#include "GameDS_DataManager.generated.h"

struct FGameDS_UnitSpawnOption;
struct FGameDS_SpawnData;
class UGameDS_SpawnUnitData;
class UDataTable;
struct FGameDS_SkillDataTable;
struct FGameDS_SkillInfoDataTable;

UCLASS(Config = Game)
class UGameDS_DataManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	template <typename DataStruct>
	const UDataTable* GetDataTable();
	template <typename DataStruct>
	const DataStruct* GetDataInRow(int32 DataID);

	const FGameDS_SpawnData* GetSpawnData(const FGameDS_UnitSpawnOption& SpawnOption) const;
	const UGameDS_HeroStatConfigData* GetHeroStatConfigData() const;
	void GetHeroStatConfigData(FGameDS_UnitStatInfo& UnitStatInfo, FGameDS_HeroStatInfo& HeroStatInfo);
	void GetHeroCreateData(FGameDS_HeroCreateInfo& CreateInfo);
	void GetEnemyStatConfigData(int32 InDataID, FGameDS_UnitStatInfo& UnitStatInfo);
	const FGameDS_SkillDataTable* GetSkillData(int32 InSkillID) const;
	const FGameDS_SkillInfoDataTable* GetSkillInfoData(int32 InSkillInfoID) const;
	const FGameDS_CollisionInfoDataTable* GetCollisionInfoData(int32 CollisionInfoID);
	FGameDS_WeaponDataTable* GetWeaponData(int32 InWeaponID) const;

	// load 시 마지막으로 저장된 serialID를 가져옴
	void InitLastSerialID(int32 InLastSerialID);
	int32 CreateItemSerialID();

	UFUNCTION(BlueprintCallable)
	void LoadHeroData(FGameDS_HeroCreateInfo& CreateInfo);
	UFUNCTION(BlueprintCallable)
	void SaveHeroData(FGameDS_HeroCreateInfo InHeroCreateInfo, bool bDefaultSetting = false);

	UFUNCTION(BlueprintCallable)
	void SaveDataWithBool(FString Section, FString Key, const bool Value);
	UFUNCTION(BlueprintCallable)
	void SaveDataWithInt(FString Section, FString Key, const int32 Value);
	UFUNCTION(BlueprintCallable)
	void SaveData(FString Section, FString Key, const TArray<FString>& Value);
	
	UFUNCTION(BlueprintCallable)
	bool LoadData(FString Section, FString Key, TArray<FString>& Value);
	UFUNCTION(BlueprintCallable)
	bool LoadDataWithBool(FString Section, FString Key, bool& Value);
	UFUNCTION(BlueprintCallable)
	bool LoadDataWithInt(FString Section, FString Key, int32& Value);

	UFUNCTION(BlueprintCallable)
	void SaveFirstData(FGameDS_HeroCreateInfo InHeroCreateInfo);

	UFUNCTION(BlueprintCallable)
	FTransform GetSpawnPointTransform();

	UFUNCTION(BlueprintCallable)
	bool IsValidSpawnPoint(int32 InSpawnPoint);

	UFUNCTION(BlueprintCallable)
	bool IsValidGameData() const;

private:
	template <typename DataStruct>
	void AddDataTable(UDataTable* DataTable);

	void InitSkillData();
	void InitSkillInfoData();

	void InitItemData();

	// DataAsset
	UPROPERTY(Config)
	FSoftClassPath SpawnUnitDataPath;
	UPROPERTY()
	UGameDS_SpawnUnitData* SpawnUnitData;

	UPROPERTY(Config)
	FSoftClassPath HeroStatConfigDataPath;
	UPROPERTY()
	UGameDS_HeroStatConfigData* HeroStatConfigData;

	UPROPERTY(Config)
	FSoftClassPath DefaultHeroSettingDataPath;
	UPROPERTY()
	UGameDS_DefaultHeroSetting* DefaultHeroSetting;

	// DataTable
	UPROPERTY(Config)
	FSoftClassPath EnemyStatConfigDataPath;
	UPROPERTY()
	UDataTable* EnemyStatConfigData;
	
	UPROPERTY(Config)
	FSoftClassPath HeroSkillDataPath;
	UPROPERTY()
	UDataTable* HeroSkillData;
	
	UPROPERTY(Config)
	FSoftClassPath EnemySkillDataPath;
	UPROPERTY()
	UDataTable* EnemySkillData;

	UPROPERTY(Config)
	FSoftClassPath HeroSkillInfoDataPath;
	UPROPERTY()
	UDataTable* HeroSkillInfoData;

	UPROPERTY(Config)
	FSoftClassPath EnemySkillInfoDataPath;
	UPROPERTY()
	UDataTable* EnemySkillInfoData;

	// common
	UPROPERTY()
	UDataTable* SkillData;
	
	UPROPERTY()
	UDataTable* SkillInfoData;
	//

	UPROPERTY(Config)
	FSoftClassPath CollisionInfoDataPath;
	UPROPERTY()
	UDataTable* CollisionInfoData;
	
	UPROPERTY(Config)
	FSoftClassPath EnemySkillSettingDataPath;
	UPROPERTY()
	UDataTable* EnemySkillSettingData;

	// common
	UPROPERTY()
	UDataTable* ItemData;
	//

	UPROPERTY(Config)
	FSoftClassPath SkillSetPath;
	UPROPERTY()
	UDataTable* SkillSetData;

	UPROPERTY(Config)
	FSoftClassPath HeroSkillSetPath;
	UPROPERTY()
	UDataTable* HeroSkillSetData;


	UPROPERTY(Config)
	FSoftClassPath SummonDataPath;
	UPROPERTY()
	UDataTable* SummonData;


	UPROPERTY(Config)
	FSoftClassPath WeaponDataPath;
	UPROPERTY()
	UDataTable* WeaponData;

	UPROPERTY(Config)
	FSoftClassPath PotionDataPath;
	UPROPERTY()
	UDataTable* PotionData;

	UPROPERTY(Config)
	FSoftClassPath CrowdControlInfoDataPath;
	UPROPERTY()
	UDataTable* CrowdControlInfoData;

	UPROPERTY(Config)
	FSoftClassPath StatusEffectInfoDataPath;
	UPROPERTY()
	UDataTable* StatusEffectInfoData;

	int32 LastItemSerialID = INDEX_NONE;
	TArray<int32> ItemSerialIDList;

	UPROPERTY()
	TMap<FName, UDataTable*> DataTableMap;

	UPROPERTY()
	FGameDS_HeroCreateInfo HeroCreateInfo;
};

template <typename DataStruct>
const UDataTable* UGameDS_DataManager::GetDataTable()
{
	auto FindData = DataTableMap.Find(DataStruct::StaticStruct()->GetFName());
	return FindData ? *FindData : nullptr;
}

template <typename DataStruct>
const DataStruct* UGameDS_DataManager::GetDataInRow(int32 DataID)
{
	const UDataTable* FindData = GetDataTable<DataStruct>();

	return FindData->FindRow<DataStruct>(FName(FString::FromInt(DataID)), TEXT(""));
}

template <typename DataStruct>
void UGameDS_DataManager::AddDataTable(UDataTable* DataTable)
{
	if (DataTable == nullptr)
		return;

	DataTableMap.Add(DataStruct::StaticStruct()->GetFName(), DataTable);
}

// DataAsset 전용
//template <typename DataClass>
//DataClass* UGameDS_DataManager::GetDataTable()
//{
//	auto FindData = DataMap.Find(DataClass::StaticClass()->GetFName());
//	return FindData ? ExactCast<DataClass>(*FindData) : nullptr;
//}
