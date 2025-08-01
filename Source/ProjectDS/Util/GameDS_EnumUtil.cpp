#include "GameDS_EnumUtil.h"

#include "ProjectDS/Unit/Controller/GameDS_InventoryController.h"

#define InvalidEnum TEXT("Invalid Enum")

FString FGameDS_EnumUtil::GetEnumToString(EGameDS_UnitType InUnitType)
{
	switch (InUnitType)
	{
		case EGameDS_UnitType::None:
			return TEXT("None");
		case EGameDS_UnitType::Hero:
			return TEXT("Hero");
		case EGameDS_UnitType::UserHero:
			return TEXT("UserHero");
		case EGameDS_UnitType::Enemy:
			return TEXT("Enemy");
		case EGameDS_UnitType::Boss:
			return TEXT("Boss");
		default: ;
	}
	return InvalidEnum;
}

FString FGameDS_EnumUtil::GetEnumToString(EGameDS_HeroClassType InUnitType)
{
	switch (InUnitType)
	{
	case EGameDS_HeroClassType::None:
		return TEXT("None");
	case EGameDS_HeroClassType::Warrior:
		return TEXT("Warrior");
	case EGameDS_HeroClassType::Magician:
		return TEXT("Magician");
	default: ;
	}
	return InvalidEnum;
}

FString FGameDS_EnumUtil::GetEnumToString(EGameDS_SkillSpawnType SpawnType)
{
	switch (SpawnType)
	{
	case EGameDS_SkillSpawnType::None:
		return TEXT("None");
	case EGameDS_SkillSpawnType::CasterForward:
		return TEXT("Player");
	case EGameDS_SkillSpawnType::WeaponSocket:
		return TEXT("WeaponSocket");
	default:;
	}
	return InvalidEnum;
}

FString FGameDS_EnumUtil::GetEnumToString(EGameDS_CollisionShape Shape)
{
	switch (Shape)
	{
	case EGameDS_CollisionShape::None:
		return TEXT("None");
	case EGameDS_CollisionShape::Box:
		return TEXT("Box");
	case EGameDS_CollisionShape::Capsule:
		return TEXT("Capsule");
	case EGameDS_CollisionShape::Sphere:
		return TEXT("Sphere");
	default:;
	}
	return InvalidEnum;
}

FString FGameDS_EnumUtil::GetEnumToString(EGameDS_WeaponSpawnType SpawnType)
{
	switch (SpawnType)
	{
	case EGameDS_WeaponSpawnType::None:
		return TEXT("None");
	case EGameDS_WeaponSpawnType::LeftWeapon:
		return TEXT("weapon_l");
	case EGameDS_WeaponSpawnType::RightWeapon:
		return TEXT("weapon_r");
	case EGameDS_WeaponSpawnType::LeftShield:
		return TEXT("shield_l");
	case EGameDS_WeaponSpawnType::RightShield:
		return TEXT("shield_r");
	default:;
	}
	return InvalidEnum;
}

FString FGameDS_EnumUtil::GetEnumToString(EGameDS_ItemType InItemType)
{
	switch (InItemType)
	{
		case EGameDS_ItemType::None:
			return TEXT("None");
		case EGameDS_ItemType::Potion:
			return TEXT("Potion");
		case EGameDS_ItemType::Weapon:
			return TEXT("Weapon");
		default: ;
	}
	return InvalidEnum;
}

FString FGameDS_EnumUtil::GetEnumToString(EGameDS_WeaponType InWeaponType)
{
	switch (InWeaponType)
	{
		case EGameDS_WeaponType::None:
			return TEXT("None");
		case EGameDS_WeaponType::Axe:
			return TEXT("Axe");
		case EGameDS_WeaponType::Sword:
			return TEXT("Sword");
		case EGameDS_WeaponType::Shield:
			return TEXT("Shield");
		default: ;
	}
	return InvalidEnum;
}

void FGameDS_EnumUtil::GetAllUnitAnimType(TArray<EGameDS_UnitAnimType>& OutAnimTypeList)
{
	const uint8 MaximumNum = StaticCast<uint8>(EGameDS_UnitAnimType::Num);
	for (uint8 Index = 0; Index < MaximumNum; Index++)
		OutAnimTypeList.Add(StaticCast<EGameDS_UnitAnimType>(Index));
}

EGameDS_EquipSlotType FGameDS_EnumUtil::GetEquipSlotType(EGameDS_ItemType InItemType, int32 Index, bool bLHand/* = false*/)
{
	uint8 SlotIndex = 0;
	switch (InItemType)
	{
		case EGameDS_ItemType::Potion:
			{
				SlotIndex = StaticCast<uint8>(EGameDS_EquipSlotType::Potion1) + Index;
			}
			break;
		case EGameDS_ItemType::Weapon:
			{
				if (bLHand)
				{
					SlotIndex = StaticCast<uint8>(EGameDS_EquipSlotType::LWeapon1) + Index;
				}
				else
				{
					SlotIndex = StaticCast<uint8>(EGameDS_EquipSlotType::RWeapon1) + Index;
				}
			}
			break;
		default: ;
	}

	return StaticCast<EGameDS_EquipSlotType>(SlotIndex);
}
