#pragma once
#include "ProjectDS/Unit/GameDS_UnitDefine.h"
#include "ProjectDS/Manager/GameDS_CustomDatas.h"
#include "ProjectDS/UI/GameDS_EquipmentWidget.h"

struct FGameDS_EnumUtil
{
	static FString GetEnumToString(EGameDS_UnitType InUnitType);
	static FString GetEnumToString(EGameDS_HeroClassType InUnitType);
	static FString GetEnumToString(EGameDS_SkillSpawnType SpawnType);
	static FString GetEnumToString(EGameDS_CollisionShape Shape);
	static FString GetEnumToString(EGameDS_WeaponSpawnType SpawnType);
	static FString GetEnumToString(EGameDS_ItemType InItemType);
	static FString GetEnumToString(EGameDS_WeaponType InWeaponType);

	static void GetAllUnitAnimType(TArray<EGameDS_UnitAnimType>& OutAnimTypeList);
	static EGameDS_EquipSlotType GetEquipSlotType(EGameDS_ItemType InItemType, int32 Index, bool bLHand = false);
};
