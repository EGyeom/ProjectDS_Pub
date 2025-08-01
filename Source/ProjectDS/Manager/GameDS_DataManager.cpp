#include "GameDS_DataManager.h"

#include "GameDS_CustomDatas.h"
#include "GameDS_UnitManager.h"
#include "../Unit/GameDS_Character.h"
#include "Kismet/GameplayStatics.h"
#include "ProjectDS/GameInstanceHolder.h"
#include "ProjectDS/ProjectDS.h"
#include "ProjectDS/UI/GameDS_EquipmentWidget.h"
#include "ProjectDS/UI/GameDS_EquipmentWidget.h"
#include "ProjectDS/UI/GameDS_EquipmentWidget.h"
#include "ProjectDS/UI/GameDS_EquipmentWidget.h"
#include "ProjectDS/UI/GameDS_EquipmentWidget.h"
#include "ProjectDS/Unit/Controller/GameDS_InventoryController.h"
#include "ProjectDS/Unit/Controller/GameDS_StatController.h"
#include "ProjectDS/Unit/Hero/GameDS_HeroCharacter.h"
#include "ProjectDS/Unit/Spawner/GameDS_HeroSpawnPoint.h"
#include "ProjectDS/Util/GameDS_EnumUtil.h"

void UGameDS_DataManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	SpawnUnitData = LoadObject<UGameDS_SpawnUnitData>(nullptr, *SpawnUnitDataPath.GetAssetPathString());
	HeroStatConfigData = LoadObject<UGameDS_HeroStatConfigData>(nullptr, *HeroStatConfigDataPath.GetAssetPathString());
	DefaultHeroSetting = LoadObject<UGameDS_DefaultHeroSetting>(nullptr, *DefaultHeroSettingDataPath.GetAssetPathString());

	EnemyStatConfigData = LoadObject<UDataTable>(nullptr, *EnemyStatConfigDataPath.GetAssetPathString());
	HeroSkillData = LoadObject<UDataTable>(nullptr, *HeroSkillDataPath.GetAssetPathString());
	EnemySkillData = LoadObject<UDataTable>(nullptr, *EnemySkillDataPath.GetAssetPathString());
	HeroSkillInfoData = LoadObject<UDataTable>(nullptr, *HeroSkillInfoDataPath.GetAssetPathString());
	EnemySkillInfoData = LoadObject<UDataTable>(nullptr, *EnemySkillInfoDataPath.GetAssetPathString());
	
	CollisionInfoData = LoadObject<UDataTable>(nullptr, *CollisionInfoDataPath.GetAssetPathString());
	EnemySkillSettingData = LoadObject<UDataTable>(nullptr, *EnemySkillSettingDataPath.GetAssetPathString());
	SkillSetData = LoadObject<UDataTable>(nullptr, *SkillSetPath.GetAssetPathString());
	HeroSkillSetData = LoadObject<UDataTable>(nullptr, *HeroSkillSetPath.GetAssetPathString());

	CrowdControlInfoData = LoadObject<UDataTable>(nullptr , *CrowdControlInfoDataPath.GetAssetPathString());
	StatusEffectInfoData = LoadObject<UDataTable>(nullptr, *StatusEffectInfoDataPath.GetAssetPathString());

	SummonData = LoadObject<UDataTable>(nullptr, *SummonDataPath.GetAssetPathString());
	WeaponData = LoadObject<UDataTable>(nullptr, *WeaponDataPath.GetAssetPathString());
	PotionData = LoadObject<UDataTable>(nullptr, *PotionDataPath.GetAssetPathString());

	InitSkillData();
	InitSkillInfoData();
	InitItemData();

	AddDataTable<FGameDS_EnemyStatConfigDataTable>(EnemyStatConfigData);
	AddDataTable<FGameDS_SkillDataTable>(SkillData);
	AddDataTable<FGameDS_SkillInfoDataTable>(SkillInfoData);
	AddDataTable<FGameDS_CollisionInfoDataTable>(CollisionInfoData);
	AddDataTable<FGameDS_EnemySkillSettingDataTable>(EnemySkillSettingData);
	AddDataTable<FGameDS_ItemDataTable>(ItemData);
	AddDataTable<FGameDS_SkillSetDataTable>(SkillSetData);
	AddDataTable<FGameDS_HeroSkillSetDataTable>(HeroSkillSetData);
	AddDataTable<FGameDS_SummonDataTable>(SummonData);
	AddDataTable<FGameDS_WeaponDataTable>(WeaponData);
	AddDataTable<FGameDS_PotionDataTable>(PotionData);
	AddDataTable<FGameDS_CrowdControlInfoDataTable>(CrowdControlInfoData);
	AddDataTable<FGameDS_StatusEffectInfoDataTable>(StatusEffectInfoData);
}

void UGameDS_DataManager::Deinitialize()
{
	Super::Deinitialize();

	SpawnUnitData = nullptr;
}

const FGameDS_SpawnData* UGameDS_DataManager::GetSpawnData(const FGameDS_UnitSpawnOption& SpawnOption) const
{
	if (SpawnOption.UnitType == EGameDS_UnitType::None)
		return nullptr;

	if (SpawnUnitData == nullptr)
		return nullptr;

	for (const FGameDS_SpawnData& Data : SpawnUnitData->SpawnDataList)
	{
		if (Data.UnitType != SpawnOption.UnitType)
			continue;
		if (Data.DataID != SpawnOption.DataID)
			continue;
		if (Data.CharacterBP.Get() == nullptr)
			break;

		return &Data;
	}

	return nullptr;
}

const UGameDS_HeroStatConfigData* UGameDS_DataManager::GetHeroStatConfigData() const
{
	return HeroStatConfigData;
}

void UGameDS_DataManager::GetHeroStatConfigData(FGameDS_UnitStatInfo& UnitStatInfo, FGameDS_HeroStatInfo& HeroStatInfo)
{
	if (HeroStatConfigData == nullptr)
		return;
	
	UnitStatInfo = HeroStatConfigData->UnitStatInfo;
	HeroStatInfo = HeroStatConfigData->HeroStatInfo;
}

void UGameDS_DataManager::GetHeroCreateData(FGameDS_HeroCreateInfo& CreateInfo)
{
	CreateInfo = HeroCreateInfo;
}

void UGameDS_DataManager::GetEnemyStatConfigData(int32 InDataID, FGameDS_UnitStatInfo& UnitStatInfo)
{
	if (EnemyStatConfigData == nullptr)
		return;

	if (const FGameDS_EnemyStatConfigDataTable* StatConfigData = EnemyStatConfigData->FindRow<FGameDS_EnemyStatConfigDataTable>(
		FName(FString::FromInt(InDataID)),TEXT("")))
	{
		UnitStatInfo = StatConfigData->UnitStatInfo;
	}

}

const FGameDS_SkillDataTable* UGameDS_DataManager::GetSkillData(int32 InSkillID) const
{
	if (SkillData == nullptr)
		return nullptr;

	if (const FGameDS_SkillDataTable* SkillTableData =
		SkillData->FindRow<FGameDS_SkillDataTable>(*FString::FromInt(InSkillID), TEXT("")))
	{
		return SkillTableData;
	}

	return nullptr;
}

const FGameDS_SkillInfoDataTable* UGameDS_DataManager::GetSkillInfoData(int32 InSkillInfoID) const
{
	if (SkillInfoData == nullptr)
		return nullptr;

	if (const FGameDS_SkillInfoDataTable* SkillInfoTableData =
		SkillInfoData->FindRow<FGameDS_SkillInfoDataTable>(*FString::FromInt(InSkillInfoID), TEXT("")))
	{
		return SkillInfoTableData;
	}

	return nullptr;
}

const FGameDS_CollisionInfoDataTable* UGameDS_DataManager::GetCollisionInfoData(int32 CollisionInfoID)
{
	if (CollisionInfoData == nullptr)
		return nullptr;

	if (const FGameDS_CollisionInfoDataTable* CollisionInfo = CollisionInfoData->FindRow<FGameDS_CollisionInfoDataTable>
		(*FString::FromInt(CollisionInfoID), TEXT("")))
	{
		return CollisionInfo;
	}

	return nullptr;
}

FGameDS_WeaponDataTable* UGameDS_DataManager::GetWeaponData(int32 InWeaponID) const
{
	if (WeaponData == nullptr)
		return nullptr;

	if (FGameDS_WeaponDataTable* Weapon = WeaponData->FindRow<FGameDS_WeaponDataTable>
		(*FString::FromInt(InWeaponID), TEXT("")))
	{
		return Weapon;
	}

	return nullptr;
}

void UGameDS_DataManager::InitLastSerialID(int32 InLastSerialID)
{
	LastItemSerialID = InLastSerialID;
}

int32 UGameDS_DataManager::CreateItemSerialID()
{
	if (ItemSerialIDList.IsEmpty())
	{
		LastItemSerialID = 1;
		return ItemSerialIDList.Add(LastItemSerialID);
	}

	return ItemSerialIDList.Add(++LastItemSerialID);
}

void UGameDS_DataManager::LoadHeroData(FGameDS_HeroCreateInfo& CreateInfo)
{
	LoadData(STAT_INI_SECTION, HEROSTATINFO_INI_KEY, CreateInfo.HeroStatList);
	LoadData(STAT_INI_SECTION, UNITSTATINFO_INI_KEY, CreateInfo.UnitStatList);
	LoadData(ITEM_INI_SECTION, EQUIPPED_LWEAPON_INI_KEY, CreateInfo.LHandList);
	LoadData(ITEM_INI_SECTION, EQUIPPED_RWEAPON_INI_KEY, CreateInfo.RHandList);
	LoadData(ITEM_INI_SECTION, EQUIPPED_POTION_INI_KEY, CreateInfo.PotionList);
	LoadData(ITEM_INI_SECTION, INVENTORY_INI_KEY, CreateInfo.InventoryItemList);
	LoadDataWithInt(SPAWN_INI_SECTION, HEROSPAWNPOINT_INI_KEY, CreateInfo.SpawnPoint);

	bool bMale = true;
	LoadDataWithBool(STAT_INI_SECTION, GENDERINFO_INI_KEY, bMale);
	CreateInfo.GenderValue = bMale ? EGameDS_GenderType::Male : EGameDS_GenderType::Female;
}

void UGameDS_DataManager::SaveHeroData(FGameDS_HeroCreateInfo InHeroCreateInfo, bool bDefaultSetting/* = false*/)
{
	if (DefaultHeroSetting == nullptr)
		return;

	if (bDefaultSetting)
	{
		InHeroCreateInfo.HeroStatList.Add(FString::FromInt(DefaultHeroSetting->HeroStatInfo.Level));
		InHeroCreateInfo.HeroStatList.Add(FString::FromInt(DefaultHeroSetting->HeroStatInfo.VitalityPoint));
		InHeroCreateInfo.HeroStatList.Add(FString::FromInt(DefaultHeroSetting->HeroStatInfo.EndurancePoint));
		InHeroCreateInfo.HeroStatList.Add(FString::FromInt(DefaultHeroSetting->HeroStatInfo.StrengthPoint));
		InHeroCreateInfo.HeroStatList.Add(FString::FromInt(DefaultHeroSetting->HeroStatInfo.IntelligencePoint));
		SaveData(STAT_INI_SECTION, HEROSTATINFO_INI_KEY, InHeroCreateInfo.HeroStatList);

		InHeroCreateInfo.UnitStatList.Add(FString::FromInt(DefaultHeroSetting->UnitStatInfo.Health));
		InHeroCreateInfo.UnitStatList.Add(FString::FromInt(DefaultHeroSetting->UnitStatInfo.Focus));
		InHeroCreateInfo.UnitStatList.Add(FString::FromInt(DefaultHeroSetting->UnitStatInfo.Stamina));
		InHeroCreateInfo.UnitStatList.Add(FString::FromInt(DefaultHeroSetting->UnitStatInfo.PhysicalDefence));
		InHeroCreateInfo.UnitStatList.Add(FString::FromInt(DefaultHeroSetting->UnitStatInfo.MagicDefence));
		SaveData(STAT_INI_SECTION, UNITSTATINFO_INI_KEY, InHeroCreateInfo.UnitStatList);

		if (InHeroCreateInfo.Weapon == EGameDS_CreateWeaponType::Sword)
		{
			InHeroCreateInfo.LHandList.Add(FString::FromInt(DefaultHeroSetting->SwordLHandWeaponDataID));
			InHeroCreateInfo.RHandList.Add(FString::FromInt(DefaultHeroSetting->SwordRHandWeaponDataID));
		}
		else
		{
			InHeroCreateInfo.LHandList.Add(FString::FromInt(DefaultHeroSetting->AxeLHandWeaponDataID));
			InHeroCreateInfo.RHandList.Add(FString::FromInt(DefaultHeroSetting->AxeRHandWeaponDataID));
		}
		
		SaveData(ITEM_INI_SECTION, EQUIPPED_LWEAPON_INI_KEY, InHeroCreateInfo.LHandList);
		SaveData(ITEM_INI_SECTION, EQUIPPED_RWEAPON_INI_KEY, InHeroCreateInfo.RHandList);

		InHeroCreateInfo.PotionList.Add(FString::FromInt(DefaultHeroSetting->PotionDataID));
		SaveData(ITEM_INI_SECTION, EQUIPPED_POTION_INI_KEY, InHeroCreateInfo.PotionList);

		for (const int32 DataID : DefaultHeroSetting->InventoryItemID)
			InHeroCreateInfo.InventoryItemList.Add(FString::FromInt(DataID));
		
		SaveData(ITEM_INI_SECTION, INVENTORY_INI_KEY, InHeroCreateInfo.InventoryItemList);
		
		SaveDataWithInt(ITEM_INI_SECTION, MAIN_LWEAPON_INDEX_INI_KEY, StaticCast<int32>(EGameDS_EquipSlotType::LWeapon1));
		SaveDataWithInt(ITEM_INI_SECTION, MAIN_RWEAPON_INDEX_INI_KEY, StaticCast<int32>(EGameDS_EquipSlotType::RWeapon1));
		SaveDataWithInt(ITEM_INI_SECTION, MAIN_POTION_INDEX_INI_KEY, StaticCast<int32>(EGameDS_EquipSlotType::Potion1));

		SaveDataWithInt(SPAWN_INI_SECTION, HEROSPAWNPOINT_INI_KEY, 0);

		const bool bMale = InHeroCreateInfo.GenderValue == EGameDS_GenderType::Male;
		SaveDataWithBool(STAT_INI_SECTION, GENDERINFO_INI_KEY, bMale);
	}
	else
	{
		// 스폰 포인트에서 저장 호출 시 여기로
		UGameDS_UnitManager* UnitManager = FGameInstanceHolder::GetSubSystem<UGameDS_UnitManager>();
		if (UnitManager == nullptr)
			return;

		AGameDS_HeroCharacter* LocalHero = UnitManager->GetLocalCharacter<AGameDS_HeroCharacter>();
		if (LocalHero == nullptr)
			return;
		
		LocalHero->GetStatController()->SaveStat();
		LocalHero->GetInventoryController()->SaveItem();

		if (LocalHero->GetLastInteractionID() != INDEX_NONE)
			LocalHero->SetSpawnPoint(LocalHero->GetLastInteractionID());
		
		SaveDataWithInt(SPAWN_INI_SECTION, HEROSPAWNPOINT_INI_KEY, LocalHero->GetSpawnPoint());
	}
}

void UGameDS_DataManager::SaveDataWithBool(FString Section, FString Key, const bool Value)
{
	FString SavePath = FPaths::ProjectConfigDir() + CONFIG_NAME;
	SavePath = FConfigCacheIni::NormalizeConfigIniPath(SavePath);

	GConfig->SetBool(*Section, *Key, Value, SavePath);
	GConfig->Flush(false, SavePath);
}

void UGameDS_DataManager::SaveDataWithInt(FString Section, FString Key, const int32 Value)
{
	FString SavePath = FPaths::ProjectConfigDir() + CONFIG_NAME;
	SavePath = FConfigCacheIni::NormalizeConfigIniPath(SavePath);

	GConfig->SetInt(*Section, *Key, Value, SavePath);
	GConfig->Flush(false, SavePath);
}

void UGameDS_DataManager::SaveData(FString Section, FString Key, const TArray<FString>& Value)
{
	FString SavePath = FPaths::ProjectConfigDir() + CONFIG_NAME;
	SavePath = FConfigCacheIni::NormalizeConfigIniPath(SavePath);
	
	GConfig->SetArray(*Section, *Key, Value, SavePath);
	GConfig->Flush(false, SavePath);
}

bool UGameDS_DataManager::LoadData(FString Section, FString Key, TArray<FString>& INIDataList)
{
	FString LoadPath = FPaths::ProjectConfigDir() + CONFIG_NAME;
	LoadPath = FConfigCacheIni::NormalizeConfigIniPath(LoadPath);

	GConfig->GetArray(*Section, *Key, INIDataList, LoadPath);

	if (INIDataList.IsEmpty())
		return false;

	return true;
}

bool UGameDS_DataManager::LoadDataWithBool(FString Section, FString Key, bool& Value)
{
	FString LoadPath = FPaths::ProjectConfigDir() + CONFIG_NAME;
	LoadPath = FConfigCacheIni::NormalizeConfigIniPath(LoadPath);

	return GConfig->GetBool(*Section, *Key, Value, LoadPath);
}

bool UGameDS_DataManager::LoadDataWithInt(FString Section, FString Key, int32& Value)
{
	FString LoadPath = FPaths::ProjectConfigDir() + CONFIG_NAME;
	LoadPath = FConfigCacheIni::NormalizeConfigIniPath(LoadPath);

	return GConfig->GetInt(*Section, *Key, Value, LoadPath);
}

void UGameDS_DataManager::SaveFirstData(FGameDS_HeroCreateInfo InHeroCreateInfo)
{
	HeroCreateInfo = InHeroCreateInfo;
}

FTransform UGameDS_DataManager::GetSpawnPointTransform()
{
	UWorld* World = FGameInstanceHolder::GetWorld();
	if (World == nullptr)
		return FTransform::Identity;

	TArray<AActor*> HeroSpawnPoints;
	UGameplayStatics::GetAllActorsOfClass(World, AGameDS_HeroSpawnPoint::StaticClass(), HeroSpawnPoints);

	if (HeroSpawnPoints.IsEmpty())
		return FTransform::Identity;

	int32 SavedSpawnPoint;
	if (LoadDataWithInt(SPAWN_INI_SECTION, HEROSPAWNPOINT_INI_KEY, SavedSpawnPoint) == false)
		SavedSpawnPoint = 0;

	for (AActor* HeroSpawnPointActor : HeroSpawnPoints)
	{
		if (HeroSpawnPointActor == nullptr)
			continue;

		AGameDS_HeroSpawnPoint* HeroSpawnPoint = Cast<AGameDS_HeroSpawnPoint>(HeroSpawnPointActor);
		if (HeroSpawnPoint == nullptr)
			continue;

		if (HeroSpawnPoint->SpawnPoint != SavedSpawnPoint)
			continue;

		return HeroSpawnPoint->GetTransform();
	}

	return FTransform::Identity;
}

bool UGameDS_DataManager::IsValidSpawnPoint(int32 InSpawnPoint)
{
	int32 SavedSpawnPoint;
	if (LoadDataWithInt(SPAWN_INI_SECTION, HEROSPAWNPOINT_INI_KEY, SavedSpawnPoint) == false)
		SavedSpawnPoint = 0;

	return SavedSpawnPoint == InSpawnPoint;
}

bool UGameDS_DataManager::IsValidGameData() const
{
	FString LoadPath = FPaths::ProjectConfigDir() + CONFIG_NAME;
	LoadPath = FConfigCacheIni::NormalizeConfigIniPath(LoadPath);

	return GConfig->FindConfigFile(LoadPath) != nullptr;
}

void UGameDS_DataManager::InitSkillData()
{
	SkillData = NewObject<UDataTable>();
	SkillData->RowStruct = FGameDS_SkillDataTable::StaticStruct();
	
	if (HeroSkillData != nullptr)
	{
		for (TMap<FName, uint8*>::TConstIterator RowMapIter(HeroSkillData->GetRowMap().CreateConstIterator()); RowMapIter; ++RowMapIter)
		{
			if (RowMapIter.Value() == nullptr)
				continue;

			FTableRowBase* TableRowBase = (FTableRowBase*)RowMapIter.Value();
			SkillData->AddRow(RowMapIter.Key(), *TableRowBase);
		}
	}
	if (EnemySkillData != nullptr)
	{
		for (TMap<FName, uint8*>::TConstIterator RowMapIter(EnemySkillData->GetRowMap().CreateConstIterator()); RowMapIter; ++RowMapIter)
		{
			if (RowMapIter.Value() == nullptr)
				continue;

			FTableRowBase* TableRowBase = (FTableRowBase*)RowMapIter.Value();
			SkillData->AddRow(RowMapIter.Key(), *TableRowBase);
		}
	}
}

void UGameDS_DataManager::InitSkillInfoData()
{
	SkillInfoData = NewObject<UDataTable>();
	SkillInfoData->RowStruct = FGameDS_SkillInfoDataTable::StaticStruct();
	
	if (HeroSkillInfoData != nullptr)
	{
		for (TMap<FName, uint8*>::TConstIterator RowMapIter(HeroSkillInfoData->GetRowMap().CreateConstIterator()); RowMapIter; ++RowMapIter)
		{
			if (RowMapIter.Value() == nullptr)
				continue;

			FTableRowBase* TableRowBase = (FTableRowBase*)RowMapIter.Value();
			SkillInfoData->AddRow(RowMapIter.Key(), *TableRowBase);
		}
	}
	
	if (EnemySkillInfoData != nullptr)
	{
		for (TMap<FName, uint8*>::TConstIterator RowMapIter(EnemySkillInfoData->GetRowMap().CreateConstIterator()); RowMapIter; ++RowMapIter)
		{
			if (RowMapIter.Value() == nullptr)
				continue;

			FTableRowBase* TableRowBase = (FTableRowBase*)RowMapIter.Value();
			SkillInfoData->AddRow(RowMapIter.Key(), *TableRowBase);
		}
	}
}

void UGameDS_DataManager::InitItemData()
{
	// weapon, potion .. 두 테이블 모두 Item data를 상속하나 실제 ItemTable에는 저장되어 있지 않는다.
	// (상속하기에 굳이 같은 데이터를 한번 더 기입할 필요 x)
	// 그런데 나중에 Item의 이름 혹은 이미지만을 필요로 할 때 따로 potion, weapon table에 접근하지 않고도 찾아올 수 있도록 하기 위함
	ItemData = NewObject<UDataTable>();
	ItemData->RowStruct = FGameDS_ItemDataTable::StaticStruct();

	if (WeaponData != nullptr)
	{
		for (TMap<FName, uint8*>::TConstIterator RowMapIter(WeaponData->GetRowMap().CreateConstIterator()); RowMapIter; ++RowMapIter)
		{
			if (RowMapIter.Value() == nullptr)
				continue;

			FTableRowBase* TableRowBase = (FTableRowBase*)RowMapIter.Value();
			ItemData->AddRow(RowMapIter.Key(), *TableRowBase);
		}
	}
	
	if (PotionData != nullptr)
	{
		for (TMap<FName, uint8*>::TConstIterator RowMapIter(PotionData->GetRowMap().CreateConstIterator()); RowMapIter; ++RowMapIter)
		{
			if (RowMapIter.Value() == nullptr)
				continue;

			FTableRowBase* TableRowBase = (FTableRowBase*)RowMapIter.Value();
			ItemData->AddRow(RowMapIter.Key(), *TableRowBase);
		}
	}
}

